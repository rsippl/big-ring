#include "sensors.h"

#include <QtCore/QVariant>

namespace {
const int FIXED_POWER_UPDATE_INTERVAL = 1000; // ms
}
namespace indoorcycling {


Sensors::Sensors(AntCentralDispatch* antCentralDispatch, QObject *parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch),
    _sensorConfigurationGroup(NamedSensorConfigurationGroup::selectedConfigurationGroup())
{
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER) {
        _updateTimer = new QTimer(this);
        _updateTimer->setInterval(FIXED_POWER_UPDATE_INTERVAL);
        connect(_updateTimer, &QTimer::timeout, this, &Sensors::sendPowerUpdate);
    }
    connect(_antCentralDispatch, &AntCentralDispatch::sensorValue, this, &Sensors::sensorValue);
}

void Sensors::initialize()
{
    for (SensorConfiguration configuration: _sensorConfigurationGroup.sensorConfigurations()) {
        _antCentralDispatch->searchForSensor(configuration.sensorType(), configuration.deviceNumber());
    }
    _updateTimer->start();
}

void Sensors::sensorValue(const SensorValueType sensorValueType,
                          const AntSensorType, const QVariant &sensorValue)
{
    switch (sensorValueType) {
    case SensorValueType::SENSOR_VALUE_HEARTRATE_BPM:
        handleHeartRate(sensorValue);
        break;
    case SensorValueType::SENSOR_VALUE_CADENCE_RPM:
        handleCadence(sensorValue);
        break;
    case SensorValueType::SENSOR_VALUE_POWER_WATT:
        handlePower(sensorValue);
        break;
    case SensorValueType::SENSOR_VALUE_WHEEL_SPEED_RPM:
        handleWheelSpeed(sensorValue);
        break;
    }
}

void Sensors::sendPowerUpdate()
{
    emit powerWattsMeasured(_sensorConfigurationGroup.fixedPower());
}

void Sensors::handleHeartRate(const QVariant &sensorValue)
{
    _heartRateBpm = sensorValue.toInt();
    emit heartRateBpmMeasured(_heartRateBpm);
}

void Sensors::handleCadence(const QVariant &sensorValue)
{
    _cadenceRpm = sensorValue.toFloat();
    emit cadenceRpmMeasured(_cadenceRpm);
}

void Sensors::handlePower(const QVariant &sensorValue)
{
    _powerWatts = sensorValue.toInt();
    emit powerWattsMeasured(_powerWatts);
}

void Sensors::handleWheelSpeed(const QVariant &sensorValue)
{
    _wheelSpeedRpm = sensorValue.toInt();
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::DIRECT_SPEED) {
        float wheelSpeedMps = _wheelSpeedRpm * 2.096 / 60.0;
        emit wheelSpeedRpmMeasured(_wheelSpeedRpm);
        emit wheelSpeedMpsMeasured(wheelSpeedMps);
    } else if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        calculatePower(_wheelSpeedRpm);
    }
}

void Sensors::calculatePower(const float)
{
    qFatal("Virtual power not implemented yet");
}
}
