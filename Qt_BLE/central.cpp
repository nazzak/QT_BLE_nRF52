#include "central.h"

central::central(QObject *parent) : QObject(parent)
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);
}

central::~central()
{
    delete m_deviceDiscoveryAgent;
    m_deviceDiscoveryAgent = Q_NULLPTR;
    qDeleteAll(m_devices);
    m_devices.clear();
}

void central::addDevice(const QBluetoothDeviceInfo &device)
{
    // If device is LowEnergy-device, add it to the list
    // -> BLE filter
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {

        m_devices.append(new DeviceInfo(device));
        qDebug() << "BLE found : " << m_devices.last()->getName() << m_devices.last()->getAddress();
    }
}

void central::slStartScanning(void)
{
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &central::addDevice);

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}
