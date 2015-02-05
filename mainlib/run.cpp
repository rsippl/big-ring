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

#include "run.h"

#include "antcontroller.h"
#include "cyclist.h"
#include "newvideowidget.h"
#include "simulation.h"

#include <QtCore/QtDebug>

Run::Run(const ANTController& antController, Simulation *simulation, RealLifeVideo& rlv, Course& course,
         NewVideoWidget *videoWidget, QObject* parent) :
    QObject(parent), _antController(antController), _rlv(rlv), _course(course),
    _simulation(simulation), _videoWidget(videoWidget)
{
    _simulation->rlvSelected(rlv);
    _simulation->courseSelected(course);

    connect(&antController, SIGNAL(heartRateMeasured(quint8)), &_simulation->cyclist(), SLOT(setHeartRate(quint8)));
    connect(&antController, SIGNAL(cadenceMeasured(quint8)), &_simulation->cyclist(), SLOT(setCadence(quint8)));
    connect(&antController, SIGNAL(powerMeasured(quint16)), &_simulation->cyclist(), SLOT(setPower(quint16)));
}

Run::~Run()
{
    // empty
}

bool Run::isRunning() const
{
    return _running;
}

void Run::start()
{
    _videoWidget->setRealLifeVideo(_rlv);
    _videoWidget->setCourse(_course);

    connect(&_simulation->cyclist(), &Cyclist::distanceChanged, _videoWidget, &NewVideoWidget::setDistance);

    connect(_videoWidget, &NewVideoWidget::readyToPlay, this, [this](bool ready) {
        if (ready) {
            qDebug() << "starting run";
            this->_simulation->cyclist().setPower(300);
            this->_simulation->play(true);
        }
    });
    _running = true;
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
