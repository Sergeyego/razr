#include "tearingmach.h"

#define _REAL_

TearingMach::TearingMach(QString devModbus, QString devTenz, QString devLas, QObject *parent) :
    QThread(parent)
{
    press = new Press(devModbus,this);
    tenz = new Tenz(devTenz,this);
    //laser = new Laser(devLas,this);
    nuIsRun=false;
    isUp=false;
    isDown=false;

    setRunning(false);

    QDir dir;
    dir.setPath(QDir::homePath()+QDir::separator()+"tenzo"+QDir::separator()+"pics");
    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);
    list=dir.entryList();
    NF=2;

    connect(press,SIGNAL(newInUp(bool)),this,SLOT(slotNewUp(bool)));
    connect(press,SIGNAL(newInDown(bool)),this,SLOT(slotNewDown(bool)));
    connect(press,SIGNAL(err(QString)),this,SIGNAL(error(QString)));
    connect(tenz,SIGNAL(err(QString)),this,SIGNAL(error(QString)));
    //connect(laser,SIGNAL(err(QString)),this,SIGNAL(error(QString)));
    connect(press,SIGNAL(newL(double)),this,SLOT(chkNu(double)));
#ifdef _REAL_
    connect(tenz,SIGNAL(newTenz(QString)),this,SIGNAL(newP(QString)));
    connect(press,SIGNAL(newL(QString)),this,SIGNAL(newL(QString)));
#endif
    connect(this,SIGNAL(finished()),this,SLOT(stop()));
    connect(this,SIGNAL(started()),this,SLOT(down()));
}

TearingMach::~TearingMach()
{
    if (this->isRunning()){
        setRunning(false);
        this->wait();
    }
    press->setPause(true);
    tenz->setPause(true);
    tenz->wait();
    //laser->setPause(true);
    //laser->wait();
}

void TearingMach::run()
{
    setRunning(true);
    double p=0, l=0, lSt=0.0;
    double t=0.0;
    int dt=333;
    double tt;

    qDebug()<<"start";

#ifdef _REAL_
    qDebug()<<"real";

#else

    if (list.size()<=NF){
        return;
    }
    QFile file(QDir::homePath()+QDir::separator()+"tenzo"+QDir::separator()+"pics"+QDir::separator()+list.at(NF));
    qDebug()<<file.fileName();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"File not open "<<list.at(NF);
        return;
    }
    if (NF<list.size()-1) NF++;
    QTextStream in(&file);
#endif

    do{

    #ifdef _REAL_
        tt=t;
        p=tenz->getP();
        l=press->getL();

    #else
        QString s=in.readLine();
        QStringList ls=s.split(" ");
        if (ls.size()>2){
            tt=ls.at(0).toDouble();
            p=ls.at(1).toDouble();
            l=ls.at(2).toDouble();
        }
    #endif

        lSt=l;
        emit newVal(tt,p,l-lSt);
        timeWait(dt);

    }while(p<5.0/1000.0 && running );

    t+=dt/1000.0;
    emit newVal(tt,p,l-lSt);

    do{

        t+=dt/1000.0;

    #ifdef _REAL_
        tt=t;
        p=tenz->getP();
        l=press->getL();

    #else
        QString s=in.readLine();
        QStringList ls=s.split(" ");
        if (ls.size()>2){
            tt=ls.at(0).toDouble();
            p=ls.at(1).toDouble();
            l=ls.at(2).toDouble();
            emit newP(QString::number(p,'f',3));
            emit newL(QString::number(l,'f',3));
        }
    #endif
        emit newVal(tt,p,l-lSt);
        timeWait(dt);

    } while(p>0.001 && running);

    qDebug()<<"stop";

#ifdef _REAL_
    qDebug()<<"real";
#else
    file.close();
#endif
}

void TearingMach::timeWait(int dt)
{
    QTime time;
    time.start();
    for(;time.elapsed() < dt;) {
    }
}

void TearingMach::slotNewUp(bool val)
{
    if (!val) {
        setRunning(false);
        nuIsRun=false;
    }
    isUp=val;
    emit newUp(val);
}

void TearingMach::slotNewDown(bool val)
{
    if (!val) {
        setRunning(false);
        nuIsRun=false;
    }
    isDown=val;
    emit newDown(val);
}

void TearingMach::reset()
{
    if (!tenz->isRunning()) tenz->start();
    //if (!laser->isRunning()) laser->start();
    press->reset();
}

void TearingMach::up()
{
    press->up();
}

void TearingMach::down()
{
    press->down();
}

void TearingMach::stop()
{
    //if (isUp || isDown)
        press->stop();
    setRunning(false);
}

void TearingMach::setNu(double val)
{
    nu=val;
}

void TearingMach::runNu()
{
    double l=press->getL();
    if (l!=nu){
        nuIsRun=true;
        l>nu? press->up() : press->down();
        press->setForce(true);
    }
}

void TearingMach::setRunning(bool val)
{
    QMutexLocker l(&mutex);
    running=val;
}

void TearingMach::chkNu(double l)
{
    //qDebug()<<nu<<"  "<<fabs(nu-l)<<" "<<l;
    if (nuIsRun && fabs(nu-l)<2.4){
        nuIsRun=false;
        press->stop();
    }
}
