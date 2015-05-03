/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
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

#include "antcontroller.h"
#include "ANT.h"
#include <QCoreApplication>

ANTController::ANTController(QObject *parent) :
    QObject(parent), _heartRate(0), _power(0), _cadence(0)
{
    initialize();
}

ANTController::~ANTController() {
    // empty
}

quint8 ANTController::heartRate() const
{
    return _heartRate;
}

quint16 ANTController::power() const
{
    return _power;
}

quint8 ANTController::cadence() const
{
    return _cadence;
}

void ANTController::foundDevice(int, int , int , QString description, QString)
{
    emit deviceFound(description);
}

void ANTController::initialize()
{
    ANT* ant = new ANT(this);
    connect(ant, SIGNAL(foundDevice(int,int,int,QString,QString)),
            SLOT(foundDevice(int,int,int,QString,QString)));
    connect(ant, &ANT::heartRateMeasured, this, &ANTController::heartRateReceived);
    connect(ant, &ANT::powerMeasured, this, &ANTController::powerReceived);
    connect(ant, &ANT::cadenceMeasured, this, &ANTController::cadenceReceived);

    ant->initialize();
}


void ANTController::heartRateReceived(int bpm)
{
    _heartRate = bpm;
    emit heartRateMeasured(bpm);
}

void ANTController::cadenceReceived(float cadence)
{
    _cadence = static_cast<int>(cadence);
    emit cadenceMeasured(_cadence);
}

void ANTController::powerReceived(float power)
{
    _power = static_cast<int>(power);
    emit powerMeasured(_power);
}

