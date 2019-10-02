#ifndef PRESS_H
#define PRESS_H

#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>
#include <QSerialPort>

class PushTimer : public QObject
{
    Q_OBJECT
public:
    PushTimer(QObject *parent = 0);
    bool value();
private:
    QTimer *timer;
    bool val;

public slots:
    void start();

private slots:
    void off();
};

class Press : public QObject
{
    Q_OBJECT
public:
    explicit Press(QString dev, QObject *parent = 0);
    ~Press();
    double getL();

private:
    bool pause;
    bool in1,in2;
    bool force;
    PushTimer *timerUp;
    PushTimer *timerDown;
    PushTimer *timerStop;
    QModbusRtuSerialMaster *modbusDevice;
    //QModbusTcpClient *modbusDevice;
    QTimer *tim;
    double L;
    
signals:
    void newInUp(bool val);
    void newInDown(bool val);
    void err(QString text);
    void newL(QString val);
    void newL(double val);

    
public slots:
    void setPause(bool p);
    void up();
    void down();
    void stop();
    void setForce(bool f);
    void readReady();
    void work();
    void reset();
    
};

#endif // PRESS_H
