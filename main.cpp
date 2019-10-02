#include "partiform.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QTextCodec *codec = QTextCodec::codecForName("Utf8");
    //QTextCodec::setCodecForTr(codec);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("neo_rtx");
    db.setHostName("192.168.1.10");
    db.setPort(5432);
    db.setUserName("user");
    db.setPassword("szsm");
    if (!db.open()) {
        QMessageBox::critical(NULL,"Error",db.lastError().text(),QMessageBox::Ok);
    }

    PartiForm w;
    w.show();
    
    return a.exec();
}
