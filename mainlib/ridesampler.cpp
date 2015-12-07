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
#include "ridesampler.h"

#include "ridefilewriter.h"
#include "run.h"

#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>


namespace {
// We sample every second.
const int SAMPLE_INTERVAL_MS = 250;
}

RideSampler::RideSampler(Run *run, RideFile &rideFile, QObject *parent):
    QObject(parent), _run(run), _rideFile(rideFile),
    _writer(new RideFileWriter(this))
{
    // make sure we don't reference _run when it's been destroyed.
    connect(_run, &QObject::destroyed, &_sampleTimer, &QTimer::stop);
    _sampleTimer.setInterval(SAMPLE_INTERVAL_MS);
    connect(&_sampleTimer, &QTimer::timeout, this, &RideSampler::takeSample);

    connect(_run, &Run::riding, this, [this]() {
        _sampleTimer.start();
        takeSample();
    });
    connect(_run, &Run::paused, &_sampleTimer, &QTimer::stop);
    connect(_run, &Run::stopped, this, &RideSampler::saveRideFile);
}

void RideSampler::takeSample()
{
    if (_run) {
        const Cyclist &cyclist = _run->cyclist();
        RideFile::Sample sample = { _run->time(),
                                    cyclist.altitude(),
                                    cyclist.cadence(),
                                    cyclist.distanceTravelled(),
                                    cyclist.heartRate(),
                                    cyclist.power(),
                                    cyclist.speed()};
        _rideFile.addSample(sample);
    }
}

void RideSampler::saveRideFile()
{
    _sampleTimer.stop();
    _writer->writeRideFile(_rideFile);
}
