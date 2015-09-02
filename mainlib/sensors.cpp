/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "sensors.h"

#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtCore/QVariant>

namespace {
const int FIXED_POWER_UPDATE_INTERVAL = 3000; // ms
}
namespace indoorcycling {


Sensors::Sensors(AntCentralDispatch* antCentralDispatch,
                 const NamedSensorConfigurationGroup &sensorConfigurationGroup,
                 QObject *parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch),
    _sensorConfigurationGroup(sensorConfigurationGroup)
{
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER ||
        _sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        _virtualPowerFunction = indoorcycling::virtualPowerFunctionForTrainer(
                    _sensorConfigurationGroup.trainer());
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
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER) {
        _updateTimer->start();
    }
}

void Sensors::sensorValue(const SensorValueType sensorValueType,
                          const AntSensorType, const QVariant &sensorValue)
{
    switch (sensorValueType) {
    case SensorValueType::HEARTRATE_BPM:
        handleHeartRate(sensorValue);
        break;
    case SensorValueType::CADENCE_RPM:
        handleCadence(sensorValue);
        break;
    case SensorValueType::POWER_WATT:
        handlePower(sensorValue);
        break;
    case SensorValueType::WHEEL_SPEED_RPM:
        handleWheelSpeed(sensorValue);
        break;
    }
}

void Sensors::sendPowerUpdate()
{
    if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::FIXED_POWER) {
        emit powerWattsMeasured(_sensorConfigurationGroup.fixedPower());
    } else if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        qDebug() << "no speed/power input, setting power to 0W.";
        _powerWatts = 0;
        emit powerWattsMeasured(_powerWatts);
    }
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
        emit wheelSpeedMpsMeasured(wheelSpeedMps);
    } else if (_sensorConfigurationGroup.simulationSetting() == SimulationSetting::VIRTUAL_POWER) {
        emit powerWattsMeasured(calculatePower(_wheelSpeedRpm));
        _updateTimer->start();
    }
}

int Sensors::calculatePower(const float wheelSpeedRpm) const
{
    float wheelCircumferenceInM = _sensorConfigurationGroup.wheelCircumferenceInMM() * 0.001;
    float wheelSpeedMps = wheelSpeedRpm * wheelCircumferenceInM / 60.0;
    float virtualPower = _virtualPowerFunction(wheelSpeedMps);
    qDebug() << QString("Virtual Power For %1 meters per second = %2W")
                .arg(wheelSpeedMps, 2, 'f')
                .arg(virtualPower, 2, 'f');
    return static_cast<int>(virtualPower);
}
}
