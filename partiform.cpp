#include "partiform.h"
#include "ui_partiform.h"

PartiForm::PartiForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PartiForm)
{
    ui->setupUi(this);

    ui->comboBoxOb->addItem(tr("Стандартный образец"));
    ui->comboBoxOb->addItem(tr("Проволока"));
    ui->comboBoxOb->setCurrentIndex(1);
    shCombo();
    ui->lineEditYear->setText(QDate::currentDate().toString("yyyy"));
    ui->groupBoxFileOb->setEnabled(false);
    ui->dateEditBegEl->setDate(QDate::currentDate().addDays(-QDate::currentDate().dayOfYear()+1));
    ui->dateEditEndEl->setDate(QDate::currentDate());
    ui->dateEditBegW->setDate(QDate::currentDate().addDays(-QDate::currentDate().dayOfYear()+1));
    ui->dateEditEndW->setDate(QDate::currentDate());
    modelMech = new QSqlQueryModel(this);
    modelMechCat = new QSqlQueryModel(this);
    updMech();
    mWindow = new MainWidget();
    modelEpart = new QSqlQueryModel(this);
    modelWpart = new QSqlQueryModel(this);

    modelMechEl = new DbTableModel("parti_mech",this);
    modelMechEl->addColumn("id_part","id_part",true,false,TYPE_INT);
    modelMechEl->addColumn("id_mech",tr("Параметр"),true,false,TYPE_STRING,NULL,new DbRelation(modelMech,0,1,this));
    modelMechEl->addColumn("kvo",tr("Значение"),false,false,TYPE_DOUBLE,new QDoubleValidator(-1000000000,1000000000,2,this));
    modelMechEl->setSort("parti_mech.id_mech");
    ui->tableViewMechEl->setModel(modelMechEl);
    ui->tableViewMechEl->setColumnHidden(0,true);
    ui->tableViewMechEl->setColumnWidth(1,250);
    ui->tableViewMechEl->setColumnWidth(2,100);

    modelMechW = new DbTableModel("wire_parti_mech",this);
    modelMechW->addColumn("id","id",true,true,TYPE_INT);
    modelMechW->addColumn("id_part","id_part",false,false,TYPE_INT);
    modelMechW->addColumn("id_mech",tr("Параметр"),false,false,TYPE_STRING,NULL,new DbRelation(modelMech,0,1,this));
    modelMechW->addColumn("value",tr("Значение"),false,false,TYPE_DOUBLE,new QDoubleValidator(-1000000000,1000000000,2,this));
    modelMechW->addColumn("id_cat",tr("Категория"),false,false,TYPE_STRING,NULL,new DbRelation(modelMechCat,0,1,this));
    modelMechW->setSort("wire_parti_mech.id_mech");
    ui->tableViewMechWire->setModel(modelMechW);
    ui->tableViewMechWire->setColumnHidden(0,true);
    ui->tableViewMechWire->setColumnHidden(1,true);
    ui->tableViewMechWire->setColumnWidth(2,210);
    ui->tableViewMechWire->setColumnWidth(3,70);
    ui->tableViewMechWire->setColumnWidth(4,120);

    refreshEparti();
    refreshWparti();
    ui->tableViewPartiEl->setModel(modelEpart);
    ui->tableViewPartiEl->setColumnHidden(0,true);
    ui->tableViewPartiEl->verticalHeader()->setDefaultSectionSize(ui->tableViewPartiEl->verticalHeader()->fontMetrics().height()*1.5);
    ui->tableViewPartiEl->verticalHeader()->hide();
    ui->tableViewPartiEl->resizeColumnsToContents();

    ui->tableViewpartiWire->setModel(modelWpart);
    ui->tableViewpartiWire->setColumnHidden(0,true);
    ui->tableViewpartiWire->verticalHeader()->setDefaultSectionSize(ui->tableViewpartiWire->verticalHeader()->fontMetrics().height()*1.5);
    ui->tableViewpartiWire->verticalHeader()->hide();
    ui->tableViewpartiWire->resizeColumnsToContents();

    connect(ui->cmdGo,SIGNAL(clicked()),this,SLOT(go()));
    connect(ui->cmdUpdEpart,SIGNAL(clicked()),this,SLOT(refreshEparti()));
    connect(ui->cmdUpdWpart,SIGNAL(clicked()),this,SLOT(refreshWparti()));
    connect(ui->tableViewPartiEl->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(updMechEl(QModelIndex)));
    connect(ui->tableViewpartiWire->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(updMechW(QModelIndex)));
    connect(ui->radioButtonNewOb,SIGNAL(toggled(bool)),ui->groupBoxNewOb,SLOT(setEnabled(bool)));
    connect(ui->radioButtonFileOb,SIGNAL(toggled(bool)),ui->groupBoxFileOb,SLOT(setEnabled(bool)));
    connect(ui->cmdReview,SIGNAL(clicked()),this,SLOT(openFileDialog()));
    connect(mWindow,SIGNAL(sigSave()),this,SLOT(saveDB()));
    connect(ui->comboBoxOb,SIGNAL(currentIndexChanged(int)),this,SLOT(shCombo()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(shCombo()));

    if (ui->tableViewPartiEl->model()->rowCount()){
        ui->tableViewPartiEl->setCurrentIndex(ui->tableViewPartiEl->model()->index(0,1));
    }

    if (ui->tableViewpartiWire->model()->rowCount()){
        ui->tableViewpartiWire->setCurrentIndex(ui->tableViewpartiWire->model()->index(0,1));
    }
}

PartiForm::~PartiForm()
{
    delete mWindow;
    delete ui;
}

void PartiForm::go()
{
    mWindow->clear();
    int n=ui->tabWidget->currentIndex();
    QString title;
    if (n==0){
        title=ui->lineEditInfoEparti->text();
    } else if (n==1){
        title=ui->lineEditInfoWparti->text();
    } else if (n==2){
        title=tr("Образец");
        if (ui->radioButtonNewOb->isChecked()){
            title=tr("П.")+ui->lineEditPart->text()+"-"+ui->lineEditYear->text();
            title+="_"+ui->lineEditMark->text()+tr("_ф")+ui->lineEditDiam->text();
        } else {
            QFile file(ui->lineEditFilePath->text());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
                qDebug()<<"File not open ";
            } else {
                QTextStream in(&file);
                while(!in.atEnd()) {
                    QString s=in.readLine();
                    QStringList ls=s.split(" ");
                    if (ls.size()>2){
                        double tt,p,l;
                        tt=ls.at(0).toDouble();
                        p=ls.at(1).toDouble();
                        l=ls.at(2).toDouble();
                        mWindow->addDataNG(tt,p,l);
                    }
                }
                mWindow->plotData();
                mWindow->calc();
                file.close();
            }
            QString name=file.fileName();
            int pos=name.length()-name.lastIndexOf(QDir::separator())-1;
            name=name.right(pos);
            if(name.right(4)==QString(".txt")) name.truncate(name.length()-4);
            title=name;
        }
    }

    mWindow->setParam(ui->doubleSpinBoxD0->value(),ui->doubleSpinBoxL0->value(),ui->doubleSpinBoxLR->value(),title, n!=2, (n==1 || ui->comboBoxOb->currentIndex()==1));
    mWindow->show();
}

void PartiForm::refreshEparti()
{
    QDate dBeg=ui->dateEditBegEl->date();
    QDate dEnd=ui->dateEditEndEl->date();
    modelEpart->setQuery("select p.id, p.n_s, e.marka, p.diam, i.nam, p.dat_part "
                         "from parti as p "
                         "inner join elrtr as e on p.id_el=e.id "
                         "inner join istoch as i on p.id_ist=i.id "
                         "where p.dat_part between '"+dBeg.toString("yyyy-MM-dd")+"' and '"+dEnd.toString("yyyy-MM-dd")+"' "
                         "order by p.dat_part, p.n_s");
    if (modelEpart->lastError().isValid()){
        QMessageBox::critical(this,"Error",modelEpart->lastError().text(),QMessageBox::Ok);
    } else {
        modelEpart->setHeaderData(1, Qt::Horizontal,tr("Партия"));
        modelEpart->setHeaderData(2, Qt::Horizontal,tr("Марка"));
        modelEpart->setHeaderData(3, Qt::Horizontal,tr("Диам."));
        modelEpart->setHeaderData(4, Qt::Horizontal,tr("Источник"));
        modelEpart->setHeaderData(5, Qt::Horizontal,tr("Дата"));
    }
}

void PartiForm::refreshWparti()
{
    QDate dBeg=ui->dateEditBegW->date();
    QDate dEnd=ui->dateEditEndW->date();
    modelWpart->setQuery("select m.id, m.n_s, w.nam, d.sdim, i.nam, m.dat "
                         "from wire_parti_m as m "
                         "inner join provol as w on m.id_provol=w.id "
                         "inner join diam as d on m.id_diam=d.id "
                         "inner join wire_source as i on m.id_source=i.id "
                         "where m.dat between '"+dBeg.toString("yyyy-MM-dd")+"' and '"+dEnd.toString("yyyy-MM-dd")+"' "
                         "order by m.dat, m.n_s");
    if (modelWpart->lastError().isValid()){
        QMessageBox::critical(this,"Error",modelWpart->lastError().text(),QMessageBox::Ok);
    } else {
        modelWpart->setHeaderData(1, Qt::Horizontal,tr("Партия"));
        modelWpart->setHeaderData(2, Qt::Horizontal,tr("Марка"));
        modelWpart->setHeaderData(3, Qt::Horizontal,tr("Диам."));
        modelWpart->setHeaderData(4, Qt::Horizontal,tr("Источник"));
        modelWpart->setHeaderData(5, Qt::Horizontal,tr("Дата"));
    }
}

void PartiForm::updMech()
{
    modelMech->setQuery("select id, nam from mech_tbl order by nam");
    if (modelMech->lastError().isValid()){
        QMessageBox::critical(this,"Error",modelMech->lastError().text(),QMessageBox::Ok);
    }
    modelMechCat->setQuery("select id, nam from mech_category order by nam");
    if (modelMechCat->lastError().isValid()){
        QMessageBox::critical(this,"Error",modelMechCat->lastError().text(),QMessageBox::Ok);
    }
}

void PartiForm::updMechEl(QModelIndex index)
{
    int id_part=ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(index.row(),0)).toInt();
    QString part=ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(index.row(),1)).toString();
    QString mark=ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(index.row(),2)).toString();
    QString diam=ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(index.row(),3)).toString();
    part+="-"+QString::number(ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(index.row(),5),Qt::EditRole).toDate().year());
    ui->lineEditInfoEparti->setText(tr("П.")+part+"_"+mark+tr("_ф")+diam);
    modelMechEl->setFilter("parti_mech.id_part = "+QString::number(id_part));
    modelMechEl->setDefaultValue(0,id_part);
    modelMechEl->select();
}

void PartiForm::updMechW(QModelIndex index)
{
    int id_part=ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),0),Qt::EditRole).toInt();
    QString part=ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),1)).toString();
    QString mark=ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),2)).toString();
    QString diam=ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),3)).toString();
    part+="-"+QString::number(ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),5)).toDate().year());
    ui->lineEditInfoWparti->setText(tr("П.")+part+"_"+mark+tr("_ф")+diam);
    modelMechW->setFilter("wire_parti_mech.id_part = "+QString::number(id_part));
    modelMechW->setDefaultValue(1,id_part);
    modelMechW->select();
    if (ui->tabWidget->currentIndex()==1){
        ui->doubleSpinBoxD0->setValue(ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(index.row(),3),Qt::EditRole).toDouble());
    }
}

void PartiForm::openFileDialog()
{
    QString fname;
    fname=QFileDialog::getOpenFileName(this,tr("Открыть файл"),
                                       QDir::homePath()+QDir::separator()+"tenzo"+QDir::separator()+QDate::currentDate().toString("yyyy"),
                                       tr("Text Documents (*.txt)") );
    ui->lineEditFilePath->setText(fname);
}

void PartiForm::saveDB()
{
    double tec=mWindow->getTek();
    double vr=mWindow->getVr();
    double ud=mWindow->getUd();
    if(!(tec && vr && ud)){
        qDebug()<<"NOT_OK";
        return;
    }
    int n=ui->tabWidget->currentIndex();
    int id_part, row;
    QSqlQuery query;
    if (n==0) {
        QMap<int,double> map;
        map.insert(1,vr);
        map.insert(2,tec);
        map.insert(3,ud);
        row=ui->tableViewPartiEl->currentIndex().row();
        id_part=ui->tableViewPartiEl->model()->data(ui->tableViewPartiEl->model()->index(row,0),Qt::EditRole).toInt();
        QMap<int, double>::const_iterator i = map.constBegin();
        while (i != map.constEnd()) {
            query.clear();
            query.prepare("delete from parti_mech where id_part= :id_part and id_mech= :id_mech ");
            query.bindValue(":id_part",id_part);
            query.bindValue(":id_mech",i.key());
            if (query.exec()){
                query.clear();
                query.prepare("insert into parti_mech (id_part, id_mech, kvo) values (:id_part, :id_mech, :kvo)");
                query.bindValue(":id_part",id_part);
                query.bindValue(":id_mech",i.key());
                query.bindValue(":kvo",QString::number(i.value(),'f',2).toDouble());
                if (!query.exec()){
                    QMessageBox::critical(this,"Error",query.lastError().text(),QMessageBox::Ok);
                }
            } else {
                QMessageBox::critical(this,"Error",query.lastError().text(),QMessageBox::Ok);
            }
            ++i;
        }
        modelMechEl->select();
    } else if (n==1){
        row=ui->tableViewpartiWire->currentIndex().row();
        id_part=ui->tableViewpartiWire->model()->data(ui->tableViewpartiWire->model()->index(row,0),Qt::EditRole).toInt();
        query.clear();
        query.prepare("delete from wire_parti_mech where id_part= :id_wparti and id_mech=1");
        query.bindValue(":id_wparti",id_part);
        if (query.exec()){
            query.clear();
            query.prepare("insert into wire_parti_mech (id_part, id_mech, value, id_cat) values (:id_wparti, 1, :value, 0)");
            query.bindValue(":id_wparti",id_part);
            query.bindValue(":value",QString::number(vr,'f',2).toDouble());
            if (!query.exec()){
                QMessageBox::critical(this,"Error",query.lastError().text(),QMessageBox::Ok);
            }

        } else {
            QMessageBox::critical(this,"Error",query.lastError().text(),QMessageBox::Ok);
        }
        modelMechW->select();
    }
}

void PartiForm::shCombo()
{
    bool standart=((ui->tabWidget->currentIndex()==2 && ui->comboBoxOb->currentIndex()==0) || ui->tabWidget->currentIndex()==0);
    if (standart){
        ui->doubleSpinBoxD0->setValue(6);
    }
    ui->doubleSpinBoxL0->setEnabled(standart);
    ui->doubleSpinBoxLR->setEnabled(standart);
}
