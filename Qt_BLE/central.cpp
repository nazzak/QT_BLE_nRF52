#include "central.h"
#include "devicehandler.h"
#include "deviceinfo.h"


central::central(QObject *parent) : QObject(parent),
    m_nRFDevice(Q_NULLPTR),
    m_nRFHandler(Q_NULLPTR)
{
    m_deviceDiscoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    // Sets the maximum search time for Bluetooth Low Energy device
    m_deviceDiscoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    
}

void central::addDevice(const QBluetoothDeviceInfo &device)
{
    // -> BLE filter : If device is LowEnergy-device, add it to the list
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        m_devicesList.append(new DeviceInfo(device));
        qDebug() << "BLE found : " << m_devicesList.last()->getName() << m_devicesList.last()->getAddress();
        
        if("nRF52_HRM_Qt" == m_devicesList.last()->getName())
        {
            m_nRFDevice = m_devicesList.last();
            qDebug() << "Youuuupi nRF in my sight.";
            
            //creating a handler if nRF device found
            m_nRFHandler = new DeviceHandler(this);
            m_nRFHandler->setDevice(m_nRFDevice);
            connect(m_nRFHandler, &DeviceHandler::sgTextToPrint, this, &central::slToView);
        }
    }
}

void central::slStartScanning(void)
{
    connect(m_deviceDiscoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &central::addDevice);

    m_deviceDiscoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void central::slToView(const QString &_str)
{
    emit sgToView(_str);
}

central::~central()
{
    if(m_deviceDiscoveryAgent)
        delete m_deviceDiscoveryAgent;
    m_deviceDiscoveryAgent = Q_NULLPTR;

    qDeleteAll(m_devicesList);
    m_devicesList.clear();

    if (m_nRFDevice)
        delete m_nRFDevice;  //not needed because algready deleted by the QList
    m_nRFDevice = Q_NULLPTR;

    if (m_nRFHandler)
        delete m_nRFHandler;
    m_nRFHandler = Q_NULLPTR;
}
