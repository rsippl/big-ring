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

#include "actuators.h"
#include "antlib/antcentraldispatch.h"
#include "bigringsettings.h"
#include "newvideowidget.h"
#include "run.h"
#include "sensorconfiguration.h"
#include "sensors.h"

#include <QtCore/QTimer>
#include <QtCore/QtDebug>

using indoorcycling::Actuators;
using indoorcycling::AntCentralDispatch;
using indoorcycling::NamedSensorConfigurationGroup;
using indoorcycling::Sensors;

Run::Run(indoorcycling::AntCentralDispatch *antCentralDispatch, RealLifeVideo& rlv, Course& course, QObject* parent) :
    QObject(parent), _antCentralDispatch(antCentralDispatch), _rlv(rlv), _course(course), _state(State::BEFORE_START)
{
    BigRingSettings settings;
   _cyclist = new Cyclist(settings.userWeight(), settings.bikeWeight(), this);

   connect(_cyclist, &Cyclist::distanceChanged, this, &Run::distanceChanged);
   connect(_cyclist, &Cyclist::speedChanged, this, &Run::speedChanged);

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

    _actuators = new indoorcycling::Actuators(_cyclist, _antCentralDispatch, sensorConfigurationGroup, this);
    connect(_simulation, &Simulation::slopeChanged, _actuators, &Actuators::setSlope);
    _actuators->initialize();

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
    // empty
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

QTime Run::time() const
{
    return _simulation->runTime();
}

void Run::start()
{

    _simulation->play(true);
    _actuators->setSlope(_rlv.slopeForDistance(_cyclist->distance()));
    _state = State::STARTING;
}

void Run::play()
{
    _simulation->play(true);
}

void Run::stop()
{
    _simulation->play(false);
    _actuators->setSlope(0.0);
    emit stopped();
}

void Run::pause()
{
    _simulation->play(false);
}

void Run::distanceChanged(float distance)
{
    if (_state == State::RIDING && distance > _course.end()) {
        setState(State::FINISHED);
    }
}

void Run::speedChanged(float speed)
{
    if ((_state == State::STARTING || _state == State::PAUSED) && speed > 0) {
        setState(State::RIDING);
    } else if (_state == State::RIDING && speed < 0.01) {
        setState(State::PAUSED);
    }
}

void Run::setState(State newState)
{
    _state = newState;
    switch(_state) {
    case State::RIDING:
        emit riding();
        break;
    case State::PAUSED:
        emit paused();
        break;
    case State::FINISHED:
        emit finished();
        break;
    default:
        // NOOP
        break;
    }
}



