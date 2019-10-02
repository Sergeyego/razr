#ifndef PARTIFORM_H
#define PARTIFORM_H

#include <QWidget>
#include "mainwidget.h"
#include <QtSql>
#include "dbtablemodel.h"

namespace Ui {
class PartiForm;
}

class PartiForm : public QWidget
{
    Q_OBJECT

public:
    explicit PartiForm(QWidget *parent = 0);
    ~PartiForm();
private slots:
    void go();
    void refreshEparti();
    void refreshWparti();
    void updMech();
    void updMechEl(QModelIndex index);
    void updMechW(QModelIndex index);
    void openFileDialog();
    void saveDB();
    void shCombo();

private:
    Ui::PartiForm *ui;
    MainWidget *mWindow;
    QSqlQueryModel *modelEpart;
    QSqlQueryModel *modelWpart;
    QSqlQueryModel *modelMech;
    QSqlQueryModel *modelMechCat;
    DbTableModel *modelMechEl;
    DbTableModel *modelMechW;
};

#endif // PARTIFORM_H
