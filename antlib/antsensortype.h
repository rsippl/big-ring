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
#ifndef ANTCHANNELTYPE_H
#define ANTCHANNELTYPE_H

#include <QtCore/QMap>
#include <QtCore/QString>

namespace indoorcycling
{
enum AntSensorType {
    SENSOR_TYPE_HR = 0x78,
    SENSOR_TYPE_POWER = 0x0B,
    SENSOR_TYPE_SPEED = 0x7B,
    SENSOR_TYPE_CADENCE = 0x7A,
    SENSOR_TYPE_SPEED_AND_CADENCE = 0x79
};

const QMap<AntSensorType,QString> ANT_SENSOR_TYPE_STRINGS =
        QMap<AntSensorType,QString>(
        {
            {SENSOR_TYPE_CADENCE, "Cadence"},
            {SENSOR_TYPE_HR, "HeartRate"},
            {SENSOR_TYPE_POWER, "Power"},
            {SENSOR_TYPE_SPEED, "Speed"},
            {SENSOR_TYPE_SPEED_AND_CADENCE,"Speed And Cadence"}
        });

enum SensorValueType {
    SENSOR_VALUE_HEARTRATE_BPM,
    SENSOR_VALUE_POWER_WATT,
    SENSOR_VALUE_CADENCE_RPM,
    SENSOR_VALUE_WHEEL_SPEED_RPM
};


const QMap<SensorValueType,QString> SENSOR_VALUE_TYPE_STRINGS =
        QMap<SensorValueType,QString>(
        {
            {SENSOR_VALUE_HEARTRATE_BPM, "BPM"},
            {SENSOR_VALUE_POWER_WATT, "W"},
            {SENSOR_VALUE_CADENCE_RPM, "RPM"},
            {SENSOR_VALUE_WHEEL_SPEED_RPM, "RPM"},
        });
}
#endif // ANTCHANNELTYPE_H
