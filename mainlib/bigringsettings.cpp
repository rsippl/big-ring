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
#include "bigringsettings.h"

namespace
{
// By default we'll average power over 3 seconds.
const int POWER_AVERAGING_MILLISECONDS_DEFAULT = 3000;
}

BigRingSettings::BigRingSettings()
{
    // empty
}

QString BigRingSettings::videoFolder() const
{
    return _settings.value("videoFolder").toString();
}

void BigRingSettings::setVideoFolder(const QString folder)
{
    _settings.setValue("videoFolder", QVariant::fromValue(folder));
}

int BigRingSettings::powerAveragingForDisplayMilliseconds() const
{
    QSettings settings;
    settings.beginGroup("display");
    const int milliseconds = settings.value("powerAveragingMilliseconds", POWER_AVERAGING_MILLISECONDS_DEFAULT).toInt();
    settings.endGroup();
    return milliseconds;
}

void BigRingSettings::setPowerAveragingForDisplayMilliseconds(const int averagingMilliseconds)
{
    QSettings settings;
    settings.beginGroup("display");
    settings.setValue("powerAveragingMilliseconds", averagingMilliseconds);
    settings.endGroup();
}
