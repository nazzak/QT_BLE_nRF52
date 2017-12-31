#include "devicehandler.h"
#include <QtEndian>
#include <QRandomGenerator>

DeviceHandler::DeviceHandler(QObject *parent) : QObject(parent)
{
    m_currentDevice = 0;
    m_foundHeartRateService = false;
    m_measuring = false;
    m_currentValue = 0;
    m_min = 0;
    m_max = 0;
    m_sum = 0;
    m_avg = 0;
    m_calories = 0;
    m_control = 0;
    m_service = 0;
}


void DeviceHandler::setDevice(DeviceInfo *device)
{
    //clearMessages();
    m_currentDevice = device;
    
    // Disconnect and delete old connection
    if (m_control) {
        m_control->disconnectFromDevice();
        delete m_control;
        m_control = 0;
    }
    
    // Create new controller and connect it if device available
    if (m_currentDevice) {
        
        // Make connections
        m_control = new QLowEnergyController(m_currentDevice->getDevice(), this);
        m_control->setRemoteAddressType(m_addressType);
        
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
        
        // Connect
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

void DeviceHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    qDebug() << "services discovered.";
    if (gatt == QBluetoothUuid(QBluetoothUuid::HeartRate)) {
        qDebug() << "Heart Rate service discovered. Waiting for service scan to be done...";
        m_foundHeartRateService = true;
    }
}

void DeviceHandler::serviceScanDone()
{
    qDebug() <<  "Service scan done.";
    
    // Delete old service if available
    if (m_service) {
        delete m_service;
        m_service = 0;
    }
    
    // If heartRateService found, create new service
    if (m_foundHeartRateService)
        m_service = m_control->createServiceObject(QBluetoothUuid(QBluetoothUuid::HeartRate), this);
    
    if (m_service) {
        connect(m_service, &QLowEnergyService::stateChanged, this, &DeviceHandler::serviceStateChanged);
        connect(m_service, &QLowEnergyService::characteristicChanged, this, &DeviceHandler::updateHeartRateValue);
        connect(m_service, &QLowEnergyService::descriptorWritten, this, &DeviceHandler::confirmedDescriptorWrite);
        m_service->discoverDetails();
    } else {
        //setError("Heart Rate Service not found.");
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
            
            const QLowEnergyCharacteristic hrChar = m_service->characteristic(QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
            if (!hrChar.isValid()) {
                qDebug() << "HR Data not found.";
                break;
            }
            
            m_notificationDesc = hrChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (m_notificationDesc.isValid())
                m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            
            break;
        }
        default:
            //nothing for now
            break;
    }
    
    emit aliveChanged();
}

void DeviceHandler::updateHeartRateValue(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    // ignore any other characteristic change -> shouldn't really happen though
    if (c.uuid() != QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement))
        return;
    
    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = data[0];
    
    //Heart Rate
    int hrvalue = 0;
    if (flags & 0x1) // HR 16 bit? otherwise 8 bit
        hrvalue = (int)qFromLittleEndian<quint16>(data[1]);
    else
        hrvalue = (int)data[1];
    
    addMeasurement(hrvalue);
}

#ifdef SIMULATOR
void DeviceHandler::updateDemoHR()
{
    int randomValue = 0;
    if (m_currentValue < 30) { // Initial value
        randomValue = 55 + QRandomGenerator::global()->bounded(30);
    } else if (!m_measuring) { // Value when relax
        randomValue = qBound(55, m_currentValue - 2 + QRandomGenerator::global()->bounded(5), 75);
    } else { // Measuring
        randomValue = m_currentValue + QRandomGenerator::global()->bounded(10) - 2;
    }
    
    addMeasurement(randomValue);
}
#endif

void DeviceHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    if (d.isValid() && d == m_notificationDesc && value == QByteArray::fromHex("0000")) {
        //disabled notifications -> assume disconnect intent
        m_control->disconnectFromDevice();
        delete m_service;
        m_service = 0;
    }
}

void DeviceHandler::disconnectService()
{
    m_foundHeartRateService = false;
    
    //disable notifications
    if (m_notificationDesc.isValid() && m_service
        && m_notificationDesc.value() == QByteArray::fromHex("0100")) {
        m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0000"));
    } else {
        if (m_control)
            m_control->disconnectFromDevice();
        
        delete m_service;
        m_service = 0;
    }
}

bool DeviceHandler::measuring() const
{
    return m_measuring;
}

bool DeviceHandler::alive() const
{
#ifdef SIMULATOR
    return true;
#endif
    
    if (m_service)
        return m_service->state() == QLowEnergyService::ServiceDiscovered;
    
    return false;
}

int DeviceHandler::hr() const
{
    return m_currentValue;
}

int DeviceHandler::time() const
{
    return m_start.secsTo(m_stop);
}

int DeviceHandler::maxHR() const
{
    return m_max;
}

int DeviceHandler::minHR() const
{
    return m_min;
}

float DeviceHandler::average() const
{
    return m_avg;
}

float DeviceHandler::calories() const
{
    return m_calories;
}

void DeviceHandler::addMeasurement(int value)
{
    m_currentValue = value;
    
    // If measuring and value is appropriate
    if (m_measuring && value > 30 && value < 250) {
        
        m_stop = QDateTime::currentDateTime();
        m_measurements << value;
        
        m_min = m_min == 0 ? value : qMin(value, m_min);
        m_max = qMax(value, m_max);
        m_sum += value;
        m_avg = (double)m_sum / m_measurements.size();
        m_calories = ((-55.0969 + (0.6309 * m_avg) + (0.1988 * 94) + (0.2017 * 24)) / 4.184) * 60 * time()/3600;
    }
    
    emit statsChanged();
}
