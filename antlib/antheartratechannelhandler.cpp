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
#include "antheartratechannelhandler.h"
#include <QtCore/QtDebug>
#include <QtCore/QTime>
#include <QtCore/QVariant>

indoorcycling::AntHeartRateChannelHandler::AntHeartRateChannelHandler(int channelNumber, QObject *parent):
    AntChannelHandler(channelNumber, SENSOR_TYPE_HR, ANT_SPORT_HR_PERIOD, parent)
{
    // empty
}

void indoorcycling::AntHeartRateChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    HeartRateMessage heartRateMessage(message.antMessage());
    if (_lastMessage.isNull() || heartRateMessage.measurementTime() != _lastMessage.measurementTime()) {
        qDebug() << QTime::currentTime().toString() << "HR:" << heartRateMessage.computedHeartRate();
        emit sensorValue(SENSOR_VALUE_HEARTRATE_BPM, sensorType(),
                         QVariant::fromValue(heartRateMessage.computedHeartRate()));
    }
    _lastMessage = heartRateMessage;
}
