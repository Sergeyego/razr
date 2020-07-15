#include "press.h"

Press::Press(QString dev, QObject *parent) :
    QObject(parent)
{
    pause=false;
    timerUp = new PushTimer(this);
    timerDown = new PushTimer(this);
    timerStop = new PushTimer(this);
    in1=false;
    in2=false;

    tim = new QTimer(this);
    tim->setInterval(200);

    modbusDevice = new QModbusRtuSerialMaster();
    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,dev);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, QSerialPort::EvenParity);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,QSerialPort::Baud9600);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,QSerialPort::Data8);
    modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,QSerialPort::OneStop);
    /*modbusDevice = new QModbusTcpClient();
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "192.168.0.2");*/
    modbusDevice->setTimeout(20);
    modbusDevice->setNumberOfRetries(3);
    modbusDevice->connectDevice();

    connect(tim,SIGNAL(timeout()),this,SLOT(work()));

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        emit err(modbusDevice->errorString());
    });

    tim->start();
}

Press::~Press()
{
    if (modbusDevice)
            modbusDevice->disconnectDevice();
    delete modbusDevice;
    qDebug()<<"close modbus";
}

double Press::getL()
{
    return L;
}

void Press::work()
{
    if (modbusDevice->state()==QModbusDevice::ConnectedState) {
        QModbusDataUnit writeRequest(QModbusDataUnit::HoldingRegisters,512,1);
        qint16 cmd=0;
        if (timerStop->value()) cmd+=1;
        if (timerDown->value()) cmd+=2;
        if (timerUp->value()) cmd+=4;
        if (force) {
            cmd+=8;
            force=false;
        }
        writeRequest.setValue(0,cmd);

        modbusDevice->sendWriteRequest(writeRequest,1);

        if (auto reply = modbusDevice->sendReadRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters,513,2),1)){
            if (!reply->isFinished()) {
                connect(reply, &QModbusReply::finished, this, &Press::readReady);
            } else {
                delete reply;
            }
        }
    }
}

void Press::reset()
{
    if (modbusDevice->state()==QModbusDevice::ConnectedState) {
        modbusDevice->disconnectDevice();
    }
    modbusDevice->connectDevice();
}


void Press::setPause(bool p)
{
    pause=p;
}

void Press::up()
{
    timerUp->start();
}

void Press::down()
{
    timerDown->start();
}

void Press::stop()
{
    timerStop->start();
    force=false;
}

void Press::setForce(bool f)
{
    force=f;
}

void Press::readReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
        if (!reply)
            return;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        reply->deleteLater();

        bool tmp1,tmp2;

        if (unit.valueCount()) {
            tmp1=( ((unit.value(0)) & ( 0x1 << (0) )) !=0 );
            tmp2=( ((unit.value(0)) & ( 0x1 << (1) )) !=0 );
        } else {
            emit err("123");
        }
        //qDebug()<<unit.value(1);
        double D=((double)unit.value(1))/100.0;
        if (L!=D){
            emit newL(QString::number(D,'f',2));
            emit newL(D);
        }
        L=D;
        //qDebug()<<D;
        if (tmp1!=in1) emit newInDown(tmp1);
        in1=tmp1;

        if (tmp2!=in2) emit newInUp(tmp2);
        in2=tmp2;

    } else {
        emit err(tr("Modbus error: ")+reply->errorString());
    }
}


PushTimer::PushTimer(QObject *parent): QObject(parent)
{
    val=false;
    timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer,SIGNAL(timeout()),this,SLOT(off()));
}

bool PushTimer::value()
{
    return val;
}

void PushTimer::off()
{
    val=false;
    timer->stop();
}

void PushTimer::start()
{
    val=true;
    timer->start();
}
