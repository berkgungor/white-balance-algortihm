#ifndef ARSERIALPORT_H
#define ARSERIALPORT_H
#include <QString>
#include <QDebug>
#include <QObject>
#include <QByteArray>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <iostream>
#include <QSerialPort>

#define CRC_BIT 0xFF
#define START_HEADER 0xE0

class ArSerialPort : public QObject
{
    Q_OBJECT

public:
    ArSerialPort();
    ~ArSerialPort();
    bool StartConnection(QString portName, int baudrate);
    bool GetConnectionStatus();
    void StopConnection();
    void Read();
    bool Write(const QByteArray &arr);
    void HandleError();
    static quint8 CalculateCheckSum(QByteArray command);

private:
    QSerialPort s;
    QQueue<QByteArray> myQueue;
};

#endif // ARSERIALPORT_H
