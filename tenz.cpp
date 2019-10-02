#include "tenz.h"

Tenz::Tenz(QString dev, QObject *parent) :
    device(dev), QThread(parent)
{
    pause=false;
    P=0.0;
}

Tenz::~Tenz()
{
    qDebug()<<"close tenz";
}

void Tenz::run()
{
    QSerialPort port;
    port.setPortName(device);
    bool ok=port.open(QIODevice::ReadWrite);
    if(ok){
        port.setBaudRate(QSerialPort::Baud19200);
        port.setDataBits(QSerialPort::Data8);
        port.setFlowControl(QSerialPort::NoFlowControl);
        port.setParity(QSerialPort::NoParity);
        port.setStopBits(QSerialPort::OneStop);
        const char c=0x10;
        while(!pause){
            //write
            bool st=false;
            for (int i=0; i<5; i++){
                port.write(&c,1);
                st=port.waitForReadyRead(300);
                if (st) {
                    port.waitForBytesWritten(100);
                    break;
                }
            }
            if (!st){
                qDebug()<<port.readAll();
                emit err(tr("Ошибка тензопреобразователя ")+device+" "+port.errorString());
                break;
            }
            //read
            double R;
            QString s=port.readAll();
            //qDebug()<<s;
            s=s.mid(1,7);
            s.replace(" ","");
            R=s.toDouble()/1000.0;
            if (R!=P){
                emit newTenz(QString::number(R,'f',3));
            }
            P=R;
            //wait
            timeWait(100);
        }
    } else {
        emit err(tr("Не удалось подключиться к тензопреобразователю ")+device);
    }
    port.close();
}

void Tenz::setPause(bool p)
{
    pause=p;
}

double Tenz::getP()
{
    return P;
}

void Tenz::timeWait(int dt)
{
    QTime time;
    time.start();
    for(;time.elapsed() < dt;) {
    }
}
