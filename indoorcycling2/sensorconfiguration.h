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
#ifndef SENSORCONFIGURATION_H
#define SENSORCONFIGURATION_H

#include <QtCore/QMap>

#include "antsensortype.h"

namespace indoorcycling {

class SensorConfiguration
{
public:
    SensorConfiguration();
    SensorConfiguration(AntSensorType sensorType, int deviceNumber);

    AntSensorType sensorType() const;
    int deviceNumber() const;
private:
    AntSensorType _sensorType;
    int _deviceNumber;
};

class NamedSensorConfigurationGroup {
public:
    /**
     * default, just creates an empty name and sensor configuration list.
     */
    NamedSensorConfigurationGroup();
    NamedSensorConfigurationGroup(const QString&name,
                                 const QMap<AntSensorType, SensorConfiguration>& sensorConfigurations);
    const QString& name() const;
    const QMap<AntSensorType, SensorConfiguration>& sensorConfigurations() const;

    static const NamedSensorConfigurationGroup selectedConfigurationGroup();
    static void saveSelectedConfigurationGroup(const QString& name);
    static QMap<QString,NamedSensorConfigurationGroup> readFromSettings();
private:
    QString _name;
    QMap<AntSensorType, SensorConfiguration> _sensorConfigurations;
};
}
#endif // SENSORCONFIGURATION_H
