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

#include "antlib/antcentraldispatch.h"
#include "newvideowidget.h"
#include "run.h"
#include "sensorconfiguration.h"
#include "sensors.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>

using indoorcycling::AntCentralDispatch;
using indoorcycling::NamedSensorConfigurationGroup;
using indoorcycling::Sensors;

Run::Run(indoorcycling::AntCentralDispatch *antCentralDispatch, RealLifeVideo& rlv, Course& course, QObject* parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch), _rlv(rlv), _course(course)
{
    QSettings settings;
    const int weight = settings.value("cyclist.weight", QVariant::fromValue(82)).toInt();
   _cyclist = new Cyclist(weight, this);

   connect(_cyclist, &Cyclist::distanceChanged, _cyclist, [this](float distance) {
       distanceChanged(distance);
   });

    NamedSensorConfigurationGroup sensorConfigurationGroup =
            NamedSensorConfigurationGroup::selectedConfigurationGroup();

   _simulation = new Simulation(sensorConfigurationGroup.simulationSetting(), *_cyclist, this);

    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    indoorcycling::Sensors* sensors = new indoorcycling::Sensors(_antCentralDispatch,
                                                                 sensorConfigurationGroup,
                                                                 this);

    connect(sensors, &Sensors::heartRateBpmMeasured, _simulation, &Simulation::setHeartRate);
    connect(sensors, &Sensors::cadenceRpmMeasured, _simulation, &Simulation::setCadence);
    connect(sensors, &Sensors::powerWattsMeasured, _simulation, &Simulation::setPower);
    connect(sensors, &Sensors::wheelSpeedMpsMeasured, _simulation, &Simulation::setWheelSpeed);
    sensors->initialize();

    _lastInformationMessage = _rlv.informationBoxForDistance(_cyclist->distance());
    _informationMessageTimer.setInterval(1000);
    connect(&_informationMessageTimer, &QTimer::timeout, this, [this]() {
        const InformationBox &informationMessage = _rlv.informationBoxForDistance(_cyclist->distance());
        if (!(informationMessage == _lastInformationMessage)) {
            _lastInformationMessage = informationMessage;
            emit newInformationMessage(informationMessage);
        }
    });
    _informationMessageTimer.start();
}

Run::~Run()
{
    _antCentralDispatch->closeAllChannels();
}

const Simulation &Run::simulation() const
{
    return *_simulation;
}

void Run::saveProgress()
{
    _rlv.setUnfinishedRun(_cyclist->distance());
    QSettings settings;
    settings.beginGroup("unfinished_runs");
    const QString key = _rlv.name();
    settings.setValue(key, QVariant::fromValue(_cyclist->distance()));
    settings.endGroup();
}

bool Run::isRunning() const
{
    return _running;
}

void Run::start()
{
    _running = true;
    _simulation->play(true);
}

void Run::play()
{
    _simulation->play(true);
}

void Run::stop()
{
    _simulation->play(false);
    emit stopped();
}

void Run::pause()
{
    _simulation->play(false);
}

void Run::distanceChanged(float distance)
{
    if (distance > _course.end()) {
        _simulation->play(false);
        emit finished();
    }
}

