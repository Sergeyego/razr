#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);
    loadSettings();
    mach = new TearingMach(devModbus, devTenz, devLas, this);

    D0=6.0;//=6.0;      //диаметр образца
    L0=5.0*D0;//=5.0*D0;   //расч.длина образца
    LR=40;//           //реальн.длина образца
    tek02=false;

    ui->lcdNumberP->display(QString::number(0,'f',3));
    ui->lcdNumberL->display(QString::number(0,'f',2));
    ui->lcdNumberdL->display(QString::number(0,'f',2));
    ui->lcdNumberT->display(QString::number(0,'f',2));

    plot = new QwtPlot(this);
    ui->verticalLayoutPlot->addWidget(plot);

    plot->setTitle(tr("Испытание образца"));
    plot->setAutoReplot(false);
    plot->setPalette(Qt::gray);
    plot->setAxisTitle(QwtPlot::xBottom,QString::fromLocal8Bit("dl, мм"));
    plot->setAxisTitle(QwtPlot::yLeft,QString::fromLocal8Bit("P, т"));
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(QPen(Qt::black,0,Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray,0,Qt::DotLine));
    grid->attach(plot);
    plot->setAxisScale(QwtPlot::yLeft,0,10);
    plot->setAxisMaxMajor(QwtPlot::xBottom,16);
    plot->setAxisMaxMajor(QwtPlot::yLeft,10);
    plot->setAxisScale(QwtPlot::xBottom,0,10);
    plot->setAxisAutoScale(QwtPlot::xBottom);
    plot->setAxisAutoScale(QwtPlot::yLeft);

    QwtPlotCanvas *canvas = new QwtPlotCanvas(plot);

    QwtPlotPicker *picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn, canvas);
    picker->setTrackerPen(QColor(Qt::blue));

    plot->setCanvas(canvas);

    curve = new QwtPlotCurve();
    QPen pen=curve->pen();
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    curve->setPen(pen);
    curve->attach(plot);

    curveEll = new QwtPlotCurve();
    curveEll->attach(plot);
    QPen penell=curveEll->pen();
    penell.setWidth(2);
    penell.setColor(Qt::red);
    QwtSymbol *symbol1 = new QwtSymbol();
    symbol1->setStyle(QwtSymbol::Ellipse);
    symbol1->setPen(penell);
    symbol1->setSize(10);
    curveEll->setSymbol(symbol1);
    curveEll->setStyle(QwtPlotCurve::Dots);

    curveLine = new QwtPlotCurve();
    QPen penline=curve->pen();
    penline.setWidth(2);
    penline.setColor(Qt::green);
    curveLine->setPen(penline);
    curveLine->attach(plot);

    curveLineUd = new QwtPlotCurve();
    curveLineUd->setPen(penline);
    curveLineUd->attach(plot);

    curveLine02 = new QwtPlotCurve();
    curveLine02->setPen(penline);
    curveLine02->attach(plot);

    curvePb = new QwtPlotCurve();
    curvePb->setPen(penline);
    curvePb->attach(plot);

    marker = new QwtPlotMarker();
    marker->attach(plot);
    marker->setLabelAlignment(Qt::AlignRight);

    mach->setNu(ui->doubleSpinBoxNu->value());

    connect(mach,SIGNAL(newDown(bool)),this,SLOT(setDown(bool)));
    connect(mach,SIGNAL(newUp(bool)),this,SLOT(setUp(bool)));
    connect(mach,SIGNAL(error(QString)),this,SLOT(showError(QString)));
    connect(ui->cmdUp,SIGNAL(clicked()),mach,SLOT(up()));
    connect(ui->cmdDown,SIGNAL(clicked()),mach,SLOT(down()));
    connect(ui->cmdBrake,SIGNAL(clicked()),mach,SLOT(stop()));
    connect(ui->checkBoxIzm,SIGNAL(clicked(bool)),this,SLOT(setIzmEnabled(bool)));
    connect(ui->cmdSave,SIGNAL(clicked()),this,SLOT(save()));
    connect(ui->cmdStart,SIGNAL(clicked()),this,SLOT(setStart()));    
    connect(ui->cmdCalc,SIGNAL(clicked()),this,SLOT(calc()));
    connect(mach,SIGNAL(newP(QString)),ui->lcdNumberP,SLOT(display(QString)));
    connect(mach,SIGNAL(newL(QString)),ui->lcdNumberL,SLOT(display(QString)));
    connect(mach,SIGNAL(finished()),this,SLOT(calc()));
    connect(mach,SIGNAL(newVal(double,double,double)),this,SLOT(addData(double,double,double)));
    connect(ui->cmdReset,SIGNAL(clicked()),this,SLOT(resetError()));
    connect(ui->cmdNu,SIGNAL(clicked()),mach,SLOT(runNu()));
    connect(ui->doubleSpinBoxNu,SIGNAL(valueChanged(double)),mach,SLOT(setNu(double)));
    connect(ui->cmdSavePart,SIGNAL(clicked(bool)),this,SIGNAL(sigSave()));
    connect(ui->cmdPrint,SIGNAL(clicked(bool)),this,SLOT(printResult()));

    mach->reset();

}

MainWidget::~MainWidget()
{
    saveSettings();
    delete curve;
    delete curveEll;
    delete curveLine;
    delete curveLine02;
    delete curvePb;
    delete ui;
}

double MainWidget::getTek()
{
    return tek*9.8;
}

double MainWidget::getVr()
{
    return vr*9.8;
}

double MainWidget::getUd()
{
    return udKn;
}

void MainWidget::show()
{
    QWidget::show();
    QRect rect = frameGeometry();
    rect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(rect.topLeft());
}

void MainWidget::setParam(double d0, double l0, double lr, QString title, bool isParti, bool isW)
{
    D0=d0;
    L0=l0;
    LR=lr;
    plot->setTitle(title);
    ui->cmdSavePart->setEnabled(isParti);
    isWire=isW;
}

void MainWidget::loadSettings()
{
    QSettings settings("szsm", "raz_05");
    double dval=settings.value("default_val").toDouble();
    ui->doubleSpinBoxNu->setValue(dval? dval : 25.0);
    devModbus=settings.value("dev_modbus",QString("/dev/ttyUSB11")).toString();
    devTenz=settings.value("dev_tenz",QString("/dev/ttyS0")).toString();
    devLas=settings.value("dev_las",QString("/dev/ttyUSB10")).toString();
}

void MainWidget::saveSettings()
{
    QSettings settings("szsm", "raz_05");
    settings.setValue("default_val", ui->doubleSpinBoxNu->value());
    settings.setValue("dev_modbus",devModbus);
    settings.setValue("dev_tenz",devTenz);
    settings.setValue("dev_las",devLas);
}

void MainWidget::showCalcs()
{
    pMx? ui->lineEditPmax->setText(QLocale().toString(pMx,'f',3)) : ui->lineEditPmax->clear();
    pKn? ui->lineEditPend->setText(QLocale().toString(pKn,'f',3)) : ui->lineEditPend->clear();
    lKn? ui->lineEditLPend->setText(QLocale().toString(lKn,'f',3)): ui->lineEditLPend->clear();
    pTk? ui->lineEditPtek->setText(QLocale().toString(pTk,'f',3)) : ui->lineEditPtek->clear();
    lTk? ui->lineEditLPtek->setText(QLocale().toString(lTk,'f',3)): ui->lineEditLPtek->clear();
    vr? ui->lineEditVr->setText(QLocale().toString(vr,'f',2)) : ui->lineEditVr->clear();

    if (tek){
        ui->lineEditTek->setText(QLocale().toString(tek,'f',3));
        if (tek02){
            ui->lineEditTek->insert(tr(" (услов.)"));
        }
    } else {
        ui->lineEditTek->clear();
    }
    //tek? ui->lineEditTek->setText(QLocale().toString(tek,'f',2)) : ui->lineEditTek->clear();

    udKn? ui->lineEditLend->setText(QLocale().toString(udKn,'f',2)) : ui->lineEditLend->clear();
    lMx? ui->lineEditLPmax->setText(QLocale().toString(lMx,'f',3)) : ui->lineEditLPmax->clear();
    udMx? ui->lineEditL->setText(QLocale().toString(udMx,'f',2)) : ui->lineEditL->clear();
}

void MainWidget::print(QPrinter *p)
{
    QwtPlotRenderer renderer;
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
    renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
    renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
    renderer.renderTo(plot,*p);
}

void MainWidget::addDataNG(double t, double p, double dl)
{
    data.x.push_back(t);
    data.y0.push_back(p);
    data.y1.push_back(dl);
}

void MainWidget::plotData()
{
    curve->setSamples(data.y1,data.y0);
    plot->replot();
}

void MainWidget::clear()
{
    data.x.clear();
    data.y0.clear();
    data.y1.clear();
    pMx=0.0;
    pKn=0.0;
    lKn=0.0;
    pTk=0.0;
    lTk=0.0;
    vr=0.0;
    tek=0.0;
    udKn=0.0;
    lMx=0.0;
    udMx=0.0;
    upr=0.0;
    pUpr=0.0;
    showCalcs();
    QVector<double> clr;
    curveEll->setSamples(clr,clr);
    curveLine->setSamples(clr,clr);
    curveLineUd->setSamples(clr,clr);
    curveLine02->setSamples(clr,clr);
    curvePb->setSamples(clr,clr);
    marker->setLabel(QwtText(""));
    plotData();
}

void MainWidget::showError(QString mess)
{
    bool b=false;
    for (int i=0; i<ui->listWidgetError->model()->rowCount(); i++){
        b=b || (ui->listWidgetError->item(i)->text()==mess);
    }
    if (!b){
        QListWidgetItem *item = new QListWidgetItem(mess,ui->listWidgetError);
        item->setBackgroundColor(QColor(230,100,100));
    }
    mach->stop();
}

void MainWidget::setStart()
{
    this->clear();
    mach->start();
}

void MainWidget::resetError()
{
    ui->listWidgetError->clear();
    mach->reset();
}

void MainWidget::addData(double t, double p, double dl)
{
    addDataNG(t,p,dl);
    plotData();
    ui->lcdNumberT->display(QString::number(t,'f',2));
    ui->lcdNumberdL->display(QString::number(dl,'f',2));
}

void MainWidget::printResult()
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    printer.setColorMode(QPrinter::Color);
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec()) {
        print(&printer);
    }
}

void MainWidget::calc()
{
    tek02=false;
    if (!data.x.size()) return;
    int Nz=data.x.size();

    bool okud(true);

    double C0x,C1x;
    double Apb,Bpb,Cpb;
    double C0,C1;
    int N0=0,N1=0,N2=0,N3=0,N4=0,N5=0,N6=0;
    bool uprOk=true;

    double *mX,*mY;
    double lP;
    int iMx, lI;
    int Nex=3; // число точек для экстраполяции
    double  c0, c1,cov00,cov01,cov11,sumsq;
    gsl_matrix *X, *cov;
    gsl_vector *y, *c;
    double chisq;

    double ll0;//=-C0/C1;
    double ll0x;// = 0.002*L0;
    mX=new double[Nz];
    mY=new double[Nz];

    //максимальная нагрузка
    pMx=data.y0[0];
    iMx=0;
    for(int i=1; i<Nz; i++){
        if(data.y0[i]>pMx) {pMx=data.y0[i]; iMx=i;}
    }
    N4=iMx;

    //определение линии упругой деформации
    lP=0.5*pMx; // точка для проведения прямой линии упругой деформации
    for(int i=0; i<Nz; i++){
        if(data.y0[i]>lP) {lP=data.y0[i]; lI=i; break;}
    }
    N6=lI;
    int N=3;//начальное к-во точек от lI;

    if (iMx<=6 || iMx>=data.y0.size()-2) {
        qDebug()<<"false!!";
        showError(tr("Невозможно выполнить вычисления для данного эксперимента. Повторите испытания."));
        return;
    }

    while(lI+N<iMx){
        for(int i=0; i<N; i++) mY[i]=data.y0[lI+i];
        for(int i=0; i<N; i++) mX[i]=data.y1[lI+i];
        gsl_fit_linear(mX,1,mY,1,N,&c0,&c1,&cov00,&cov01,&cov11,&sumsq);
        int sg=1, md=1;
        double ext;
        for(int i=N; i<N+Nex; i++){
            ext=c0+c1*data.y1[lI+i] - data.y0[lI+i];
           sg = sg && (ext>0.0);
           //md = md && (fabs(ext[N-i])>eps);
           md = md && (ext/data.y0[lI+i]>0.04);
        }
        if (sg && md) break;
        N++;
    }
    N2=lI+N;

    N1=lI-1;
    if (data.y0.size()<N1 || data.y0.size()<N2){
        qDebug()<<"false!!";
        showError(tr("Невозможно выполнить вычисления для данного эксперимента. Повторите испытания."));
        return;
    } else {
        for(int i=N1;i>0;i--){
            for(int j=i; j<=N2; j++) mY[j-i]=data.y0[j];
            for(int j=i; j<=N2; j++) mX[j-i]=data.y1[j];
            gsl_fit_linear(mX,1,mY,1,N2-i+1,&c0,&c1,&cov00,&cov01,&cov11,&sumsq);
            int Ne=i-Nex; if(Ne<0) Ne=0;
            int sg=1, md=1;
            double ext;
            for(int j=Ne; j<i; j++){
                ext=data.y0[j] - c0-c1*data.y1[j];
               sg = sg && (ext>0.0);
               md = md && (ext/data.y0[j]>0.05);
            }
            if (sg && md) {N1=i; break;}
        }
        if(N1==lI-1)N1=0;
        C0=c0; C1=c1;
    }

    N0=3; //для параболы (предел текучести)
    N3=iMx-N0;
    c = gsl_vector_alloc (N0);
    cov = gsl_matrix_alloc (N0, N0);

    for(int i=N3; i>=N2; i--){
        int n = iMx-i+1;
        X = gsl_matrix_alloc (n, N0);
        y = gsl_vector_alloc (n);
        for(int j=0; j<n; j++){
            gsl_matrix_set (X, j, 0, 1.0);
            gsl_matrix_set (X, j, 1, data.y1[i+j]);
            gsl_matrix_set (X, j, 2, data.y1[i+j]*data.y1[i+j]);
            gsl_vector_set (y, j, data.y0[i+j]);
        }
        gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (n, 3);
        gsl_multifit_linear (X, y, c, cov,&chisq, work);
        gsl_multifit_linear_free (work);
        gsl_matrix_free(X);
        gsl_vector_free(y);
        int sg=1, md=1;
        double ext;
        //fprintf(stderr,"%d ",i);
        for(int j=i-Nex; j<i; j++){
            ext=data.y0[j] -
           (gsl_vector_get(c,0)+gsl_vector_get(c,1)*data.y1[j]+
            gsl_vector_get(c,2)*data.y1[j]*data.y1[j]);
           sg = sg && (ext>0.0);
           md = md && (ext/data.y0[j]>0.006);
           //fprintf(stderr,"%f ",ext);
        }
        //fprintf(stderr,"\n");
        if (sg && md) {N3=i; break;}
   }

   if (N3==iMx-N0){
       tek02=true;
   }

   ll0=-C0/C1;
   ll0x = 0.002*L0; //0.2%
   C1x=C1;
   C0x=C0-ll0x*C1x;
   //получены коэффициенты параболы
   Cpb=gsl_vector_get(c,0);
   Bpb=gsl_vector_get(c,1);
   Apb=gsl_vector_get(c,2);

   //расчет удлинения. находим точку разрыва N5

   /*Nex=2;
   for(int i=N4+N0; i<Nz-1; i++){
       int n = i-N4;
       X = gsl_matrix_alloc (n, N0);
       y = gsl_vector_alloc (n);
       for(int j=0; j<n; j++){
           gsl_matrix_set (X, j, 0, 1.0);
           gsl_matrix_set (X, j, 1, data.y1[N4+j]);
           gsl_matrix_set (X, j, 2, data.y1[N4+j]*data.y1[N4+j]);
           gsl_vector_set (y, j, data.y0[N4+j]);
       }
       gsl_multifit_linear_workspace * work = gsl_multifit_linear_alloc (n, 3);
       gsl_multifit_linear (X, y, c, cov,&chisq, work);
       gsl_multifit_linear_free (work);
       gsl_matrix_free(X);
       gsl_vector_free(y);
       int sg=1, md=1;
       double ext;
       //fprintf(stderr,"%d ",i);

       if((Nex+i)>=Nz){
           qDebug()<<"false Nex+i";
           qDebug()<<Nex<<" "<<Nex+i<<" "<<Nz;
           //showError(tr("Невозможно выполнить вычисления для данного эксперимента. Повторите испытания."));
           //return;
           okud=false;
           lKn=0.0;
           lTk=0.0;
           udKn=0.0;
           lMx=0.0;
           udMx=0.0;
           break;
       }

       for(int j=i+1; j<Nex+i+1; j++){
           ext= - data.y0[j] +
                   (gsl_vector_get(c,0)+gsl_vector_get(c,1)*data.y1[j]+
                    gsl_vector_get(c,2)*data.y1[j]*data.y1[j]);
           sg = sg && (ext>0.0);
           md = md && (ext/data.y0[j]>0.1);
           //fprintf(stderr,"%f ",ext);
       }
       //fprintf(stderr,"\n");
       N5=i;
       if (sg && md) break;
   }
   //fprintf(stderr,"%d %d %f\n",Nz,N5,pW->getData()->data()[N5].y[0]);
   gsl_matrix_free(cov);
   gsl_vector_free(c);

   delete[] mX;
   delete[] mY;
    */

   N5=N4;

   double max=0;
   for (int i=N4+1; i<Nz-2; i++){
       if (fabs(data.y0[i]-data.y0[i+1])>=max){
           N5=i;
           max=fabs(data.y0[i]-data.y0[i+1]);
       }
   }

   pMx=data.y0[N4];
   pKn=data.y0[N5];
   lMx=data.y1[N4]+C0/C1;
   lKn=data.y1[N5]+C0/C1;
   pUpr=data.y0[N2];

   if(tek02){
       for(int i=N6; i<N4;i++){
           double p1=C0x+C1x*data.y1[i];
           double p2=data.y0[i];
           N3=i;
           if(p1>p2) break;
       }
   }

   pTk=data.y0[N3];
   if (pUpr>pTk) {
       pUpr=pTk;
       uprOk=false;
   }

   lTk=data.y1[N3]+C0/C1;
   vr=pMx*1000.0/(3.14159*D0*D0/4.0);
   tek=pTk*1000.0/(3.14159*D0*D0/4.0);
   upr=pUpr*1000.0/(3.14159*D0*D0/4.0);

   if (okud && !isWire) {
       lKn=(lKn-lMx)+lMx*L0/LR;//1 часть - удлин.за счет шейки; 2 часть - пропорц.удлин
       lMx=lMx*L0/LR;
       lTk=lTk*L0/LR;
       udMx=lMx/L0*100.0;
       udKn=lKn/L0*100.0;

   }

   //qDebug()<<N0<<" "<<N1<<" "<<N2<<" "<<N3<<" "<<N4<<" "<<N5<<" "<<N6;

   showCalcs();

   QVector<double> xell,yell;
   //xell.push_back(data.y1[N0]);
   //yell.push_back(data.y0[N0]);
   //xell.push_back(data.y1[N1]);
   //yell.push_back(data.y0[N1]);
   if(uprOk){
       xell.push_back(data.y1[N2]);
       yell.push_back(data.y0[N2]);
   }
   xell.push_back(data.y1[N3]);
   yell.push_back(data.y0[N3]);
   xell.push_back(data.y1[N4]);
   yell.push_back(data.y0[N4]);
   //if (okud && !isWire){
       xell.push_back(data.y1[N5]);
       yell.push_back(data.y0[N5]);
   //}
   //xell.push_back(data.y1[N6]);
   //yell.push_back(data.y0[N6]);
   curveEll->setSamples(xell,yell);

   QVector<double> xline,yline;
   double yl=data.y0[N3]+(data.y0[N4]-data.y0[N3])/2.0;
   xline.push_back(-C0/C1);
   yline.push_back(0);
   xline.push_back((yl-C0)/C1);
   yline.push_back(yl);
   curveLine->setSamples(xline,yline);

   /*QVector<double> xlineUd,ylineUd;
   double x0=-C0/C1;
   double x1=(data.y0[N5]-C0)/C1;
   double x2=data.y1[N5];
   qDebug()<<x0<<" "<<x1<<" "<<x2<<" ud = "<<(x2-x1)*100.0/L0<<"%";
   xlineUd.push_back(x2-x1+x0);
   ylineUd.push_back(0);
   xlineUd.push_back(data.y1[N5]);
   ylineUd.push_back(data.y0[N5]);
   curveLineUd->setSamples(xlineUd,ylineUd);*/

   //qDebug()<<"Модуль упругости"<<(C1*1000*9.81*LR/(3.14159*D0*D0/4.0))/1000<<"ГПа";

   QVector<double> xline02, yline02;
   if (tek02){
       double yl02=data.y0[N3]+(data.y0[N4]-data.y0[N3])/2.0;
       xline02.push_back(-C0x/C1x);
       yline02.push_back(0);
       xline02.push_back((yl02-C0x)/C1x);
       yline02.push_back(yl02);
   }
   curveLine02->setSamples(xline02, yline02);

   QVector<double> xpb,ypb;
   for (int i=N2; i<=N4; i++){
       xpb.push_back(data.y1[i]);
       ypb.push_back(Cpb+Bpb*data.y1[i]+Apb*data.y1[i]*data.y1[i]);
   }
   curvePb->setSamples(xpb,ypb);
   updTextMarker();
   plot->replot();
}

void MainWidget::setUp(bool val)
{
    ui->labelUp->setPixmap(val? QPixmap(":/on.png") : QPixmap(":/off.png"));
    setLock(val);
}

void MainWidget::setDown(bool val)
{
    ui->labelDown->setPixmap(val? QPixmap(":/on.png") : QPixmap(":/off.png"));
    setLock(val);
}

void MainWidget::setLock(bool val)
{
    ui->cmdUp->setDisabled(val);
    ui->cmdDown->setDisabled(val);
    ui->cmdStart->setDisabled(val);
    ui->cmdCalc->setDisabled(val);
    ui->cmdNu->setDisabled(val);
    ui->doubleSpinBoxNu->setDisabled(val);
    ui->cmdSave->setDisabled(val);
}

void MainWidget::setIzmEnabled(bool val)
{
    curveEll->setVisible(val);
    curveLine->setVisible(val);
    curveLineUd->setVisible(val);
    curveLine02->setVisible(val);
    curvePb->setVisible(val);
    marker->setVisible(val);
    plot->replot();
}

void MainWidget::save()
{
    QDir dir;
    dir.setPath(QDir::homePath()+QDir::separator()+"tenzo");
    if (!dir.exists()) dir.mkdir(dir.path());
    dir.setPath(dir.path()+QDir::separator()+QString::number(QDate::currentDate().year()));
    if (!dir.exists()) dir.mkdir(dir.path());
    QString name=plot->title().text();
    name.replace(".","-");
    name.replace("/","-");
    name.replace(" ","_");
    QString pth=dir.path()+QDir::separator()+name;
    QString sNam=QFileDialog::getSaveFileName(NULL,tr("Записать график в файл"),pth,tr("pdf (*.pdf)"));
    if(sNam.length()>0){
        if(sNam.right(4)==QString(".pdf")) sNam.truncate(sNam.length()-4);
        QFile file(sNam+".txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
            QTextStream out(&file);
            for (int i=0; i<data.x.size(); i++){
                out<<QString::number(data.x.at(i),'f',6)<<" "<<QString::number(data.y0.at(i),'f',6)<<" "<<QString::number(data.y1.at(i),'f',6)<<"\n";
            }
            file.close();
        }
        QPrinter printer(QPrinter::HighResolution);
        printer.setOrientation(QPrinter::Landscape);
        printer.setColorMode(QPrinter::Color);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(sNam+".pdf");
        print(&printer);
    }
}

void MainWidget::updTextMarker()
{
    QwtText text;
    QFont font("Times", 13, QFont::Normal);
    text.setFont(font);
    int Nz=data.x.size();
    double vpp=0.0;
    if (Nz>1) vpp=(data.y1[Nz-1]-data.y1[1])/(data.x[Nz-1]-data.x[0]);
    QString str;
    str=(tr("Машина: УМЭ-10ТМ\n"
                    "Скорость: ")+QLocale().toString(vpp,'f',2)+tr(" мм/с")+"\n");
    str+=tr("Образец: диам.=");
    str+=QLocale().toString(D0,'f',1)+tr(" мм");
    if (!isWire){
        str+=(", расч.длина=");
        str+=QLocale().toString(L0,'f',2)+tr(" мм");
    }
    str+="\n";
    str+=tr("Дата испытаний: ")+QDateTime::currentDateTime().toString("dd.MM.yy hh:mm")+"\n";
    str+=tr("Предел пропорц.= ")+QLocale().toString(upr,'f',2)+tr(" кгс/мм2 (")+QLocale().toString(upr*9.8,'f',2)+tr(" МПа)")+"\n";
    str+=tr("Предел текуч.");
    if (tek02){
        str+=tr(" (усл.)");
    }
    str+=tr("= ")+QLocale().toString(tek,'f',2)+tr(" кгс/мм2 (")+QLocale().toString(tek*9.8,'f',2)+tr(" МПа)")+"\n";
    str+=tr("Предел проч.= ")+QLocale().toString(vr,'f',2)+tr(" кгс/мм2 (")+QLocale().toString(vr*9.8,'f',2)+tr(" МПа)")+"\n";
    if (!ui->lineEditLend->text().isEmpty()){
        str+=tr("Отн. удлинен. после разрыва= ")+ui->lineEditLend->text()+"%";
    }
    text.setText(str);
    text.setColor(Qt::black);
    double x=plot->axisScaleDiv(QwtPlot::xBottom).range();
    double y=plot->axisScaleDiv(QwtPlot::yLeft).range();
    marker->setXValue(x*0.25);
    marker->setYValue(y*0.3);
    marker->setLabel(text);
}
