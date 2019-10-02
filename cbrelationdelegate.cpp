#include "cbrelationdelegate.h"
#include <QLineEdit>

CbRelationDelegate::CbRelationDelegate(QObject *parent)
               : QItemDelegate(parent)
{

}

QWidget *CbRelationDelegate::createEditor (QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{

    const DbTableModel *sqlModel = qobject_cast<const DbTableModel *>(index.model());
    if (!sqlModel) return QItemDelegate::createEditor(parent, option, index);
    QWidget *editor=NULL;
    if (sqlModel->relation(index.column())){
        QAbstractItemModel *childModel=sqlModel->relation(index.column())->model();
        if (!childModel)
            return QItemDelegate::createEditor(parent, option, index);
        QComboBox *combo = new QComboBox(parent);
        combo->setModel(childModel);
        combo->setModelColumn(sqlModel->relation(index.column())->columnDisplay());
        combo->setEditable(true);
        CustomCompletter *c = new CustomCompletter(combo);
        c->setModel(childModel);
        c->setCompletionColumn(sqlModel->relation(index.column())->columnDisplay());
        combo->setCompleter(c);
        //combo->completer()->setCompletionMode(QCompleter::PopupCompletion);
        editor=combo;
    } else {
        switch (sqlModel->columnType(index.column())){
            case TYPE_BOOL:
            {
                editor=NULL;
                break;
            }
            case TYPE_INTBOOL:
            {
                editor=NULL;
                break;
            }
            case TYPE_STRING:
            {
                QLineEdit *edt=new QLineEdit(parent);
                if (sqlModel->validator(index.column()))
                    edt->setValidator(sqlModel->validator(index.column()));
                editor=edt;
                break;
            }
            case TYPE_INT:
            {
                editor=new DoubleLineEdit(sqlModel->validator(index.column()),parent);
                break;
            }
            case TYPE_DOUBLE:
            {
                editor=new DoubleLineEdit(sqlModel->validator(index.column()),parent);
                break;
            }
            case TYPE_DATE:
            {
                QDateEdit *dateEdit = new QDateEdit(parent);
                dateEdit->setCalendarPopup(true);
                QCalendarWidget * pCW = new QCalendarWidget(parent);
                pCW->setFirstDayOfWeek( Qt::Monday );
                dateEdit->setCalendarWidget( pCW );
                editor=dateEdit;
                break;
            }
            default:
            {
                editor=new QLineEdit(parent);
                break;
            }
        }
    }
    if (editor) editor->installEventFilter(const_cast<CbRelationDelegate *>(this));
    return editor;
}

void CbRelationDelegate::setEditorData ( QWidget * editor, const QModelIndex & index ) const
{
    const DbTableModel *sqlModel = qobject_cast<const DbTableModel *>(index.model());
    if (!sqlModel || !sqlModel->relation(index.column())){
        return QItemDelegate::setEditorData(editor, index);
    } else {
        QComboBox *combo = qobject_cast<QComboBox *>(editor);
        if (!combo)
            return QItemDelegate::setEditorData(editor, index);
        int pos=combo->findText(sqlModel->data(index).toString());
        combo->setCurrentIndex(pos);
    }
}

void CbRelationDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
    if (!index.isValid())
        return;
    DbTableModel *sqlModel = qobject_cast<DbTableModel *>(model);
    if (!sqlModel) {
        return QItemDelegate::setModelData(editor, model, index);
    }
    if (sqlModel->relation(index.column())){
        QAbstractItemModel *childModel = sqlModel->relation(index.column())->model();
        QComboBox *combo = qobject_cast<QComboBox *>(editor);
        if (!childModel || !combo) {
            return QItemDelegate::setModelData(editor, model, index);
        }
        int pos=combo->findText(combo->currentText(),Qt::MatchFixedString);
        if (pos!=-1) combo->setCurrentIndex(pos);
        int currentRow = combo->currentIndex();
        int childEditIndex = sqlModel->relation(index.column())->columnKey();
        QVariant val=childModel->data(childModel->index(currentRow, childEditIndex), Qt::EditRole);
        sqlModel->setData(index,val,Qt::EditRole);
    } else
        return QItemDelegate::setModelData(editor, model, index);
}

void CbRelationDelegate::updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex& /* index */) const
{
    editor->setGeometry(option.rect);
}

bool CbRelationDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type()== QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key()==Qt::Key_Tab || keyEvent->key()==Qt::Key_Down || keyEvent->key()==Qt::Key_Up){
            //qDebug()<<"Key_Tab";
            QWidget *editor = qobject_cast<QWidget*>(object);
            emit commitData(editor);
            emit closeEditor(editor);
            return false;
        } else return QItemDelegate::eventFilter(object,event);
    }
    return QItemDelegate::eventFilter(object,event);
}

DoubleLineEdit::DoubleLineEdit(QValidator *validator, QWidget *parent) :
    QLineEdit(parent)
{
    if (validator) {
        this->setValidator(validator);
    }
}

void DoubleLineEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->text()==",") insert("."); else
        QLineEdit::keyPressEvent(e);
}

CustomCompletter::CustomCompletter(QObject *parent):QCompleter(parent)
{
    setCompletionMode(QCompleter::PopupCompletion);
}

bool CustomCompletter::eventFilter(QObject *o, QEvent *e)
{
    /*if (e->type()==QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key()==Qt::Key_Tab) {
            this->popup()->close();
            return false;
        }
    }*/
    return QCompleter::eventFilter(o,e);
}
