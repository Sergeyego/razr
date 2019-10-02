#ifndef LASER_H
#define LASER_H

#include <QThread>
#include <QDebug>
#include <QWaitCondition>
#include <QMutex>
#include <QtSerialPort/QSerialPort>
#include <QTime>

class Laser : public QThread
{
    Q_OBJECT
public:
    explicit Laser(QString dev, QObject *parent = 0);
    ~Laser();
    void run();
    void setPause(bool p);
    double getL();
private:
    bool pause;
    double L;
    QString device;

    
signals:
    void newL(QString val);
    void newL(double val);
    void err(QString text);
    
public slots:
    
};

#endif // LASER_H
