#ifndef QCOMBODELEGATE_H
#define QCOMBODELEGATE_H
#include <QtGui>
#include <QtSql>
#include "dbtablemodel.h"
#include <QDateEdit>
#include <QCompleter>
#include <QLineEdit>
#include <QCalendarWidget>

class CbRelationDelegate : public QItemDelegate
{
    Q_OBJECT
public:
     CbRelationDelegate(QObject *parent = 0);
     QWidget *createEditor(
                 QWidget *parent,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const;
     void setEditorData(QWidget *editor,
                        const QModelIndex &index) const;
     void setModelData(QWidget *editor,
                       QAbstractItemModel *model,
                       const QModelIndex &index) const;
     void updateEditorGeometry(
             QWidget *editor,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const;
     bool eventFilter(QObject *object, QEvent *event);
 };

class DoubleLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit DoubleLineEdit(QValidator *validator, QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *e);
};

class CustomCompletter : public QCompleter
{
    Q_OBJECT
public:
    CustomCompletter(QObject *parent=0);
    bool eventFilter(QObject *o, QEvent *e);

};

#endif // QCOMBODELEGATE_H
