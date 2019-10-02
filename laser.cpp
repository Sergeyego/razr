#include "laser.h"

Laser::Laser(QString dev, QObject *parent) :
    device(dev), QThread(parent)
{
    pause=false;
    L=0.0;
}

Laser::~Laser()
{
    qDebug()<<"close laser";
}

void Laser::run()
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
        const char c=0x01;
        while(!pause){
            //write
            bool st;
            for (int i=0; i<3; i++){
                port.write(&c,1);
                st=port.waitForReadyRead(100);
                if (st) {
                    port.waitForBytesWritten(300);
                    break;
                }
            }
            if (!st){
                emit err(tr("Ошибка лазера ")+device);
                break;
            }
            //read
            QString s=port.readAll();
            if (s.length()>=2) {
                s.truncate(s.length()-1);
            }
            double R=s.toDouble();
            //qDebug()<<R;
            double D=350.0-((double)350.0-220.0)/(62768-2768)*(R-2768.0);
            if (L!=D){
                emit newL(QString::number(D,'f',2));
                emit newL(D);
            }
            L=D;
            QTime time;
            time.start();
            for(;time.elapsed() < 100;) {
            }
        }
    } else {
        emit err(tr("Не удалось подключиться к лазеру ")+device);
    }
    port.close();
}

void Laser::setPause(bool p)
{
    pause=p;
}

double Laser::getL()
{
    return L;
}
