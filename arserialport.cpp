#include "arserialport.h"
//#include <unistd.h>
#include "QDebug"


#define TV_DELAY_WAIT_FOR_BYTES_WRITTEN_IN_SEC 1
#define TV_DELAY_WAIT_FOR_BYTES_READ_IN_SEC 1

ArSerialPort::ArSerialPort()
{
}

ArSerialPort::~ArSerialPort()
{
    if(GetConnectionStatus())
    {
        StopConnection();
    }
}

bool ArSerialPort::StartConnection(QString portName, int baudrate)
{
    s.setPortName(portName);
    s.setBaudRate(static_cast<QSerialPort::BaudRate>(baudrate));
    qDebug() << "Name :" << portName;
    qDebug() << "Baudrate :" << baudrate;

    if(GetConnectionStatus())
    {
        return false;
    }

    if (!s.open(QIODevice::ReadWrite))
    {
        qDebug() << "not open";
        return false;
    }
    else
    {
        qDebug() << "open";
        QObject::connect(&s,&QSerialPort::readyRead,this,&ArSerialPort::Read);
    }
    return true;
}

void ArSerialPort::StopConnection()
{
    if(GetConnectionStatus())
    {
        s.close();
    }
}

bool ArSerialPort::GetConnectionStatus()
{
    return s.isOpen();
}

void ArSerialPort::Read()
{
    const QByteArray data = s.readAll();
    qDebug() << data;
}

bool ArSerialPort::Write(const QByteArray &arr)
{
    QString strResponse;
    qDebug() << arr;
    bool bDataSent = false;
//    s.write(arr);

    if(s.write(arr) != -1)
    {
        if(s.waitForBytesWritten(TV_DELAY_WAIT_FOR_BYTES_WRITTEN_IN_SEC))
        {
            bDataSent = true;
                    // read response
                    if (s.waitForReadyRead(90))
                    {
                        QByteArray responseData = s.readAll();
                        while (s.waitForReadyRead(15))
                            responseData += s.readAll();

                        strResponse = responseData;
                        bDataSent = strResponse.contains("OK");

                    }
                    else
                    {
                        strResponse = "Wait read response timeout";
                    }

        }
        else
        {
            qDebug() << "Write request timeout";
        }
    }

    return bDataSent;
}

void ArSerialPort::HandleError()
{
    qDebug() << "TO DO!";
}

quint8 ArSerialPort::CalculateCheckSum(QByteArray cCommandArray)
{
    quint8 sum = 0;

    for(int i = 0;i< cCommandArray.count();i++)  //Count from beginning to length - 1(CRC)
    {
         sum += cCommandArray.at(i);
    }
    return (CRC_BIT - sum);
}
