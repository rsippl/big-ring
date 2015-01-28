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

namespace {
const int TIMER_INTERVAL = 100; // ms
}
ANTController::ANTController(QObject *parent) :
    QObject(parent), _heartRate(0), _power(0), _cadence(0), antThread(new QThread(this)),
    antTimer(new QTimer(this))
{
    antTimer->setInterval(TIMER_INTERVAL);
    connect(antThread, &QThread::finished, this, &ANTController::finished);
    initialize();
}

ANTController::~ANTController() {
    if (antThread->isRunning()) {
        qWarning("Call quit() before destroying ANTController");
    }
}

bool ANTController::isRunning() const
{
    return antThread->isRunning();
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
    ANT* ant = new ANT;
    ant->moveToThread(antThread);

    connect(antThread, SIGNAL(started()), ant, SLOT(initialize()));

    connect(antTimer, SIGNAL(timeout()), ant, SLOT(readCycle()));
    connect(ant, SIGNAL(initializationSucceeded()), antTimer, SLOT(start()));
    connect(ant, SIGNAL(foundDevice(int,int,int,QString,QString)), SLOT(foundDevice(int,int,int,QString,QString)));
    connect(ant, SIGNAL(heartRateMeasured(quint8)), SLOT(heartRateReceived(quint8)));
    connect(ant, SIGNAL(powerMeasured(float)), SLOT(powerReceived(float)));
    connect(ant, SIGNAL(cadenceMeasured(float)), SLOT(cadenceReceived(float)));

    antThread->start();
}


void ANTController::heartRateReceived(quint8 bpm)
{
    _heartRate = bpm;
    emit heartRateMeasured(bpm);
}

void ANTController::cadenceReceived(float cadence)
{
    _cadence = static_cast<quint8>(cadence);
    emit cadenceMeasured(_cadence);
}

void ANTController::quit()
{
    antThread->quit();
}

void ANTController::powerReceived(float power)
{
    _power = static_cast<quint16>(power);
    emit powerMeasured(_power);
}

