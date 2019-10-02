#include "viewer.h"

Viewer::Viewer(QWidget *parent) :
    QTableView(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    //setSelectionBehavior(QAbstractItemView::SelectRows);
    menuEnabled=true;
    verticalHeader()->setDefaultSectionSize(verticalHeader()->fontMetrics().height()*1.5);
    updAct = new QAction(tr("Обновить"),this);
    removeAct = new QAction(tr("Удалить"),this);
    this->setAutoScroll(true);
    this->setItemDelegate(new CbRelationDelegate(this));

    connect(updAct,SIGNAL(triggered()),this,SLOT(upd()));
    connect(removeAct,SIGNAL(triggered()),this,SLOT(remove()));
}

void Viewer::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    connect(this->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(submit(QModelIndex,QModelIndex)));
}

void Viewer::setColumnsWidth(QVector<int> width)
{
    for (int i=0; i<width.size(); i++){
        setColumnWidth(i,width[i]);
    }
}


void Viewer::keyPressEvent(QKeyEvent *e)
{
    DbTableModel *sqlModel = qobject_cast<DbTableModel *>(this->model());
    if (sqlModel && this->editTriggers()!=QAbstractItemView::NoEditTriggers){
        int c=sqlModel->rowCount();
        int row=currentIndex().row();

        switch (e->key()){
            case Qt::Key_Delete:
            {
                if (e->modifiers()==Qt::ControlModifier) remove();
                break;
            }
            case Qt::Key_Escape:
            {
                sqlModel->escAdd();
                break;
            }
            case Qt::Key_Down:
            {
                if ((row==c-1) || !c){
                    sqlModel->insertRow(sqlModel->rowCount());
                    scrollToBottom();
                }
                QTableView::keyPressEvent(e);
                break;
            }
            case Qt::Key_Tab:
            {
                int j=sqlModel->columnCount()-1;
                bool hidden=isColumnHidden(j);
                while (hidden && j>0){
                    j--;
                    hidden=this->isColumnHidden(j);
                }

                int i=0;
                hidden=isColumnHidden(i);
                while (hidden && i<sqlModel->columnCount()){
                    i++;
                    hidden=this->isColumnHidden(i);
                }

                if ((currentIndex().column()==j) && (row==sqlModel->rowCount()-1)) {
                    sqlModel->insertRow(sqlModel->rowCount());
                    setCurrentIndex(this->model()->index(sqlModel->rowCount()-1,i));
                } else
                    this->setCurrentIndex(this->moveCursor(this->MoveNext,Qt::NoModifier));
                break;
            }
            default:
            {
                QTableView::keyPressEvent(e);
                break;
            }
        }

    } else
        QTableView::keyPressEvent(e);
}


void Viewer::upd()
{
    DbTableModel *sqlModel = qobject_cast<DbTableModel *>(this->model());
    if (sqlModel) sqlModel->select();
}

void Viewer::remove()
{
    DbTableModel *sqlModel = qobject_cast<DbTableModel *>(this->model());
    QModelIndex ind=this->currentIndex();
    if (sqlModel && sqlModel->rowCount() && sqlModel->removeRow(ind.row()))
        setCurrentIndex(model()->index(ind.row()-1,ind.column()));
}

void Viewer::submit(QModelIndex ind, QModelIndex oldInd)
{
    if (this->editTriggers()==QAbstractItemView::NoEditTriggers) return;
    DbTableModel *sqlModel = qobject_cast<DbTableModel *>(this->model());
    if (sqlModel){
        if (sqlModel->isAdd() && !sqlModel->isEdt() && oldInd.row()!=sqlModel->rowCount()-2)
            sqlModel->escAdd();
        else if ((sqlModel->isEdt() && !sqlModel->isAdd()) || (sqlModel->isAdd() && ind.row()!=sqlModel->rowCount()-1))
            if (!sqlModel->submitRow()){
                //this->selectionModel()->reset();
                //disconnect(this->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(submit(QModelIndex,QModelIndex)));
                //this->selectionModel()->blockSignals(true);
                //this->blockSignals(true);
                //this->selectionModel()->clear();
                //this->selectionModel()->reset();
                //setCurrentIndex(oldInd);
                //this->selectionModel()->reset();
                //this->selectionModel()->select(oldInd,QItemSelectionModel::ClearAndSelect);
                //this->blockSignals(false);
                //this->selectionModel()->setCurrentIndex(oldInd,QItemSelectionModel::ClearAndSelect);
                //this->selectionModel()->blockSignals(false);
                //connect(this->selectionModel(),SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),this,SLOT(submit(QModelIndex,QModelIndex)));
            }
    }
}

void Viewer::focusOutEvent(QFocusEvent *event)
{
    if (this->editTriggers()!=QAbstractItemView::NoEditTriggers && event->reason()==Qt::MouseFocusReason){
        DbTableModel *sqlModel = qobject_cast<DbTableModel *>(this->model());
        if (sqlModel && sqlModel->isAdd() && !sqlModel->isEdt()) sqlModel->escAdd();
    }
    return QTableView::focusOutEvent(event);
}

void Viewer::setMenuEnabled(bool value)
{
    menuEnabled=value;
}

void Viewer::contextMenuEvent(QContextMenuEvent *event)
{
    if (menuEnabled){
        QMenu menu(this);
        menu.addAction(updAct);
        menu.addSeparator();
        menu.addAction(removeAct);
        menu.addSeparator();
        menu.exec(event->globalPos());
    }
}


DateEdit::DateEdit(QWidget *parent): QDateEdit(parent)
{
    this->setCalendarPopup(true);
    QCalendarWidget * pCW = new QCalendarWidget(this);
    pCW->setFirstDayOfWeek( Qt::Monday );
    this->setCalendarWidget( pCW );
}
