#ifndef CENTRAL_H
#define CENTRAL_H

#include "defines.h"
#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QList>
#include "deviceinfo.h"

class central : public QObject
{
    Q_OBJECT
public:
    explicit central(QObject *parent = nullptr);
    ~central();

public:
    QBluetoothDeviceDiscoveryAgent * m_deviceDiscoveryAgent;
    QList<DeviceInfo *> m_devices;

signals:

public slots:
    void addDevice(const QBluetoothDeviceInfo &device);
    void slStartScanning(void);
};

#endif // CENTRAL_H
