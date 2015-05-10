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

namespace indoorcycling
{
enum AntSensorType {
    SENSOR_TYPE_UNUSED = 0x00,
    SENSOR_TYPE_HR = 0x78,
    SENSOR_TYPE_POWER = 0x0B,
    SENSOR_TYPE_SPEED = 0x7B,
    SENSOR_TYPE_CADENCE = 0x7A,
    SENSOR_TYPE_SPEED_AND_CADENCE = 0x79
};

enum SensorValueType {
    SENSOR_VALUE_HEARTRATE_BPM,
    SENSOR_VALUE_POWER_WATT,
    SENSOR_VALUE_CADENCE_RPM,
    SENSOR_VALUE_WHEEL_SPEED_RPM
};
}
#endif // ANTCHANNELTYPE_H
