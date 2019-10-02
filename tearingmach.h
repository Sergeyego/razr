#ifndef TEARINGMACH_H
#define TEARINGMACH_H

#include <QThread>
#include "tenz.h"
#include "laser.h"
#include "press.h"
#include <QFile>
#include <QDir>
#include <math.h>

class TearingMach : public QThread
{
    Q_OBJECT
public:
    explicit TearingMach(QString devModbus, QString devTenz, QString devLas, QObject *parent = 0);
    ~TearingMach();
    void run();

private:
    Tenz *tenz;
    //Laser *laser;
    Press *press;
    void timeWait(int dt);
    int NF;
    QStringList list;
    bool running;
    QMutex mutex;
    double nu;
    bool nuIsRun;
    bool isUp;
    bool isDown;

signals:
    void newUp(bool val);
    void newDown(bool val);
    void error(QString mess);
    void newP(QString val);
    void newL(QString val);
    void newVal(double t, double p, double dl);

private slots:
    void slotNewUp(bool val);
    void slotNewDown(bool val);

public slots:
    void reset();
    void up();
    void down();
    void stop();
    void setNu(double val);
    void runNu();

private slots:
    void setRunning(bool val);
    void chkNu(double l);

};

#endif // TEARINGMACH_H
