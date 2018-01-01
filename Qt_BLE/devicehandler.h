#ifndef DEVICEHANDLER_H
#define DEVICEHANDLER_H

#include <QDateTime>
#include <QVector>
#include <QTimer>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include "deviceinfo.h"


class DeviceHandler : public QObject
{
    Q_OBJECT

public:
    DeviceHandler(QObject *parent = 0);
    
    QLowEnergyController *m_control;
    
    QLowEnergyService * m_service;
    QLowEnergyService * m_serviceBatt;
    QLowEnergyService * m_serviceDeviceInfo;
    
    QLowEnergyDescriptor m_notificationDesc, m_notificationBatt;
    DeviceInfo *m_currentDevice;
    
    // Statistics
    QDateTime m_start;
    QDateTime m_stop;
    
    QVector<int> m_measurements;
    QLowEnergyController::RemoteAddressType m_addressType = QLowEnergyController::PublicAddress;
    
public:
    
    void setDevice(DeviceInfo *device);
    bool measuring() const;
    bool alive() const;
    
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    
    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                                  const QByteArray &value);
    
    bool m_foundHeartRateService;
    bool m_foundDeviceInfoService;
    bool m_foundBatteryService;
    bool m_measuring;
    int m_currentValue, m_min, m_max, m_sum;
    float m_avg, m_calories;

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();
    void sgTextToPrint(const QString &_str);

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

};

#endif // DEVICEHANDLER_H
