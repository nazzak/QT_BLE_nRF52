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
    central(const central& _a);
    central& operator =(const central& _a);

    ~central();

public:
    QBluetoothDeviceDiscoveryAgent * m_deviceDiscoveryAgent;
    QList<DeviceInfo *> m_devicesList;

private:
    DeviceInfo * m_nRFDevice;
    DeviceHandler * m_nRFHandler;
    
signals:
    void sgToView(const QString & _str);
    
    
public slots:
    void addDevice(const QBluetoothDeviceInfo &device);
    void slStartScanning(void);
    void slToView(const QString &_str);
};

#endif // CENTRAL_H
