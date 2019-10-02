#ifndef TENZ_H
#define TENZ_H

#include <QThread>
#include <QDebug>
#include <QTime>
#include <QtSerialPort/QSerialPort>


class Tenz : public QThread
{
    Q_OBJECT
public:
    explicit Tenz(QString dev, QObject *parent = 0);
    ~Tenz();
    void run();
    void setPause(bool p);
    double getP();

private:
    bool pause;
    double P;
    QString device;
    void timeWait(int dt);
    
signals:
    void newTenz(QString val);
    void err(QString text);
    
public slots:
    
};

#endif // TENZ_H
