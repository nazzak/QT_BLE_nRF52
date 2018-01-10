#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include "defines.h"
#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>



class DeviceInfo: public QObject
{
    Q_OBJECT

public:
    DeviceInfo(QObject *parent = nullptr);
    DeviceInfo(const QBluetoothDeviceInfo &d);
    QString getAddress() const;
    QString getName() const;
    QBluetoothDeviceInfo getDevice();
    void setDevice(const QBluetoothDeviceInfo &dev);

signals:
    void deviceChanged();

private:
    QBluetoothDeviceInfo device;
};

#endif // DEVICEINFO_H
