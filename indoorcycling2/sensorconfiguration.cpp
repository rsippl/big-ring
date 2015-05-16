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
#include "sensorconfiguration.h"

#include <QtCore/QSettings>

namespace indoorcycling {


SensorConfiguration::SensorConfiguration()
{
    // empty
}

SensorConfiguration::SensorConfiguration(AntSensorType sensorType, int deviceNumber):
    _sensorType(sensorType), _deviceNumber(deviceNumber)
{
    // empty
}

AntSensorType SensorConfiguration::sensorType() const
{
    return _sensorType;
}

int SensorConfiguration::deviceNumber() const
{
    return _deviceNumber;
}

NamedSensorConfigurationGroup::NamedSensorConfigurationGroup()
{
    // empty
}

NamedSensorConfigurationGroup::NamedSensorConfigurationGroup(
        const QString &name, const QMap<AntSensorType, SensorConfiguration> &sensorConfigurations):
    _name(name), _sensorConfigurations(sensorConfigurations)
{
    // empty
}

const QString &NamedSensorConfigurationGroup::name() const
{
    return _name;
}

const QMap<AntSensorType,SensorConfiguration>& NamedSensorConfigurationGroup::sensorConfigurations() const
{
    return _sensorConfigurations;
}

const NamedSensorConfigurationGroup NamedSensorConfigurationGroup::selectedConfigurationGroup()
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    QString name = settings.value("selectedConfiguration").toString();

    return readFromSettings().value(name);
}

void NamedSensorConfigurationGroup::saveSelectedConfigurationGroup(const QString &name)
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    return settings.setValue("selectedConfiguration", QVariant::fromValue(name));
}

QMap<QString, NamedSensorConfigurationGroup> NamedSensorConfigurationGroup::readFromSettings()
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.beginGroup("configurations");
    const QStringList configurationNames = settings.childGroups();

    QMap<QString,NamedSensorConfigurationGroup> namedConfigurationGroups;
    for (QString configurationName: configurationNames) {
        QMap<AntSensorType,SensorConfiguration> sensorConfigurations;
        settings.beginGroup(configurationName);
        int size = settings.beginReadArray("sensors");
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            AntSensorType sensorType =
                    static_cast<AntSensorType>(settings.value("sensorType").toInt());
            int deviceNumber = settings.value("deviceNumber").toInt();
            sensorConfigurations.insert(sensorType, SensorConfiguration(sensorType, deviceNumber));
        }
        settings.endArray();
        settings.endGroup();
        NamedSensorConfigurationGroup group(configurationName, sensorConfigurations);
        namedConfigurationGroups[configurationName] = group;
    }
    settings.endGroup();

    return namedConfigurationGroups;
}

}
