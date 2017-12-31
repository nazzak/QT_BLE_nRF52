#ifndef CENTRAL_H
#define CENTRAL_H

#include "defines.h"
#include <QObject>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QList>


class DeviceHandler;
class DeviceInfo;

class central : public QObject
{
    Q_OBJECT
public:
    explicit central(QObject *parent = nullptr);
    ~central();

public:
    QBluetoothDeviceDiscoveryAgent * m_deviceDiscoveryAgent;
    QList<DeviceInfo *> m_devices;
    QLowEnergyController * m_controller;
    
    DeviceInfo * m_nRFDevice;
    DeviceHandler * m_nRFHandler;

public slots:
    void addDevice(const QBluetoothDeviceInfo &device);
    void slStartScanning(void);
};

#endif // CENTRAL_H
