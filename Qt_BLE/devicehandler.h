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
    QLowEnergyService *m_service;
    QLowEnergyDescriptor m_notificationDesc;
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

    // Statistics
    int hr() const;
    int time() const;
    float average() const;
    int maxHR() const;
    int minHR() const;
    float calories() const;
    
    //QLowEnergyController
    void serviceDiscovered(const QBluetoothUuid &);
    void serviceScanDone();
    
    //QLowEnergyService
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void updateHeartRateValue(const QLowEnergyCharacteristic &c,
                              const QByteArray &value);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                                  const QByteArray &value);
    
    void addMeasurement(int value);
    bool m_foundHeartRateService;
    bool m_measuring;
    int m_currentValue, m_min, m_max, m_sum;
    float m_avg, m_calories;

signals:
    void measuringChanged();
    void aliveChanged();
    void statsChanged();

public slots:
    void startMeasurement();
    void stopMeasurement();
    void disconnectService();

};

#endif // DEVICEHANDLER_H
