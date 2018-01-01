#include "devicehandler.h"
#include <QtEndian>
#include <QRandomGenerator>

DeviceHandler::DeviceHandler(QObject *parent) : QObject(parent)
{
    m_currentDevice = 0;
    m_foundHeartRateService = false;
    m_foundDeviceInfoService = false;
    m_foundBatteryService = false;
    m_measuring = false;
    m_currentValue = 0;
    m_min = 0;
    m_max = 0;
    m_sum = 0;
    m_avg = 0;
    m_calories = 0;
    m_control = 0;
    m_service = m_serviceBatt = m_serviceDeviceInfo = 0;
}


void DeviceHandler::setDevice(DeviceInfo *device)
{
    //clearMessages();
    m_currentDevice = device;
    
    // Disconnect and delete old connection
    if (m_control)
    {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = 0;
    }
    
    // Create new controller and connect it if device available
    if (m_currentDevice)
    {
        
        // Create the controller
        m_control = new QLowEnergyController(m_currentDevice->getDevice(), this);
        //m_control->setRemoteAddressType(m_addressType);
        
        // Create the connections
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &DeviceHandler::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceHandler::serviceScanDone);
        connect(m_control, static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this]() {
                    qDebug() << "Cannot connect to remote device .";
                });
        
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            qDebug() << "Controller connected. Search services...";
            m_control->discoverServices();
        });

        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            qDebug() << "LowEnergy controller disconnected";
        });
        
        // Connect and wait
        m_control->connectToDevice();
    }
}

void DeviceHandler::startMeasurement()
{
    if (alive()) {
        m_start = QDateTime::currentDateTime();
        m_min = 0;
        m_max = 0;
        m_avg = 0;
        m_sum = 0;
        m_calories = 0;
        m_measuring = true;
        m_measurements.clear();
        emit measuringChanged();
    }
}

void DeviceHandler::stopMeasurement()
{
    m_measuring = false;
    emit measuringChanged();
}

// Service search
void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "services discovered.";
    
    if (gatt == QBluetoothUuid(QBluetoothUuid::HeartRate)) {
        qDebug() << "Heart Rate service discovered. Waiting for service scan to be done...";
        m_foundHeartRateService = true;
    }
    if (gatt == QBluetoothUuid(QBluetoothUuid::DeviceInformation)) {
        qDebug() << "Device Information service discovered. Waiting for service scan to be done...";
        m_foundDeviceInfoService = true;
    }
    if (gatt == QBluetoothUuid(QBluetoothUuid::BatteryService)) {
        qDebug() << "Battery service discovered. Waiting for service scan to be done...";
        m_foundBatteryService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    qDebug() << "Service scan done.";
    
    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = 0;
    }
    if (m_serviceBatt) {
        delete m_serviceBatt;
        m_serviceBatt = 0;
    }
    if (m_serviceDeviceInfo) {
        delete m_serviceDeviceInfo;
        m_serviceDeviceInfo = 0;
    }
    
    // If heartRateService found, create new service
    if (m_foundHeartRateService)
        m_service = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::HeartRate), this);
    
    // If BatteryService found, create new service
    if (m_foundBatteryService)
        m_serviceBatt = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::BatteryService), this);
    
    // If DeviceService found, create new service
    if (m_foundDeviceInfoService)
        m_serviceDeviceInfo = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::DeviceInformation), this);
    
    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateValue);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);

        // Start discovering the details : sub services, characteristics, ...
        m_service->discoverDetails();
    } else {
        qDebug() << "Heart Rate Service not found.";
    }
    
    if (m_serviceBatt) {
        connect(m_serviceBatt, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_serviceBatt, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateValue);
        connect(m_serviceBatt, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        
        // Start discovering the details : sub services, characteristics, ...
        m_serviceBatt->discoverDetails();
    } else {
        qDebug() << "Battery Service not found.";
    }
    
    if (m_serviceDeviceInfo) {
        connect(m_serviceDeviceInfo, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_serviceDeviceInfo, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateValue);
        connect(m_serviceDeviceInfo, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        
        // Start discovering the details : sub services, characteristics, ...
        m_serviceDeviceInfo->discoverDetails();
    } else {
        qDebug() << "Device Service not found.";
    }
}

// Service functions
void DeviceHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
        case QLowEnergyService::DiscoveringServices:
            qDebug() << "Discovering services...";
            break;
        case QLowEnergyService::ServiceDiscovered:
        {
            qDebug() << "Service discovered.";
            
            // Configure the notification on the HR characteristic
            const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
            if (hrChar.isValid()) {
                m_notificationDesc = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (m_notificationDesc.isValid())
                    m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            }
            
            // Configure the notification on the Battery characteristic
            const QLowEnergyCharacteristic battChar = m_serviceBatt->characteristic(QBluetoothUuid(QBluetoothUuid::BatteryLevel));
            if (battChar.isValid()) {
                m_notificationBatt = battChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
                if (m_notificationBatt.isValid())
                    m_serviceBatt->writeDescriptor(m_notificationBatt, QByteArray::fromHex("0100"));
            }
            
            // Configure Device Info characteristic
            const QLowEnergyCharacteristic DeviceInfoChar = m_serviceDeviceInfo->characteristic(QBluetoothUuid(QBluetoothUuid::ManufacturerNameString));
            if (DeviceInfoChar.isValid()) {
                //
            }

            break;
        }
        default:
            //nothing for now
            break;
    }
    
    emit aliveChanged();
}

void DeviceHandler::updateValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = data[0];
    
    if (c.uuid() == QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement))
    {
    
    //Heart Rate
    int hrvalue = 0;
    if (flags & 0x1) // HR 16 bit? otherwise 8 bit
        hrvalue = (int)qFromLittleEndian<quint16>(data[1]);
    else
        hrvalue = (int)data[1];
    
    qDebug() << "hrvalue : " << hrvalue;
        emit sgTextToPrint("hrvalue : " + QString::number(hrvalue));
    }
    
    if (c.uuid() == QBluetoothUuid(QBluetoothUuid::BatteryLevel))
    {
        qDebug() << "Battvalue : " << data[0];
        emit sgTextToPrint("Battvalue : " + QString::number(data[0]));
    }
    
    
    // Testing
    QLowEnergyCharacteristic DeviceChar = m_serviceDeviceInfo->characteristic(QBluetoothUuid(QBluetoothUuid::ManufacturerNameString));
    qDebug() << "Device name : " << DeviceChar.value();
    emit sgTextToPrint("Device name : " + DeviceChar.value());

}

// The signal descriptorWritten is emitted when the value of descriptor is successfully changed to newValue
void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && value == QByteArray::fromHex("0000") && (d == m_notificationBatt || d == m_notificationDesc)) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        delete m_serviceBatt;
        m_service = m_serviceBatt = Q_NULLPTR;
    }
}

void DeviceHandler::disconnectService()
{
    m_foundHeartRateService = false;
    m_foundDeviceInfoService = false;
    m_foundBatteryService = false;
    
    //disable notifications
    if (m_notificationDesc.isValid() &&
        m_notificationBatt.isValid() &&
        m_service && m_serviceBatt &&
        m_notificationDesc.value() == QByteArray::fromHex("0100") &&
        m_notificationBatt.value() == QByteArray::fromHex("0100"))
    {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();
        
        delete m_service;
        delete m_serviceBatt;
        m_service = m_serviceBatt = Q_NULLPTR;
    }
}

bool DeviceHandler::measuring() const
{
    return m_measuring;
}

bool DeviceHandler::alive() const
{
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;

    return false;
}
