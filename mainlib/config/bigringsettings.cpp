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

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QUuid>

namespace
{
// By default we'll average power over 3 seconds.
const int POWER_AVERAGING_MILLISECONDS_DEFAULT = 3000;

const qreal USER_WEIGHT_KILOGRAMS_DEFAULT = 75.0;
const qreal BIKE_WEIGHT_KILOGRAMS_DEFAULT = 10.0;
}

BigRingSettings::BigRingSettings()
{
    // empty
}

QString BigRingSettings::videoFolder() const
{
    return _settings.value("videoFolder").toString();
}

void BigRingSettings::setVideoFolder(const QString &folder)
{
    _settings.setValue("videoFolder", QVariant::fromValue(folder));
}

QString BigRingSettings::tcxFolder() const
{
    if (_settings.contains("tcxSaveFolder")) {
        return _settings.value("tcxSaveFolder").toString();
    } else {
        const QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        const QString directoryName = QString("Big-Ring/Rides");

        return documentsPath + "/" + directoryName;
    }
}

void BigRingSettings::setTcxFolder(const QString &folder)
{
    _settings.setValue("tcxSaveFolder", QVariant::fromValue(folder));
}

qreal BigRingSettings::userWeight() const
{
    return _settings.value("user.weight", QVariant::fromValue(USER_WEIGHT_KILOGRAMS_DEFAULT)).toDouble();
}

void BigRingSettings::setUserWeight(const qreal userWeight)
{
    _settings.setValue("user.weight", QVariant::fromValue(userWeight));
}

qreal BigRingSettings::bikeWeight() const
{
    return _settings.value("bike.weight", QVariant::fromValue(BIKE_WEIGHT_KILOGRAMS_DEFAULT)).toDouble();
}

void BigRingSettings::setBikeWeight(const qreal bikeWeight)
{
    _settings.setValue("bike.weight", QVariant::fromValue(bikeWeight));
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

Qt::AspectRatioMode BigRingSettings::videoAspectRatio() const
{
    QSettings settings;
    settings.beginGroup("display");

    Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatioByExpanding;
    QString aspectRatioModeString = settings.value("aspectRatioMode", "KeepAspectRatioByExpanding").toString();
    if (aspectRatioModeString == "KeepAspectRatio") {
        aspectRatioMode = Qt::KeepAspectRatio;
    }
    settings.endGroup();

    return aspectRatioMode;
}

void BigRingSettings::setVideoAspectRatio(Qt::AspectRatioMode aspectRatioMode)
{
    _settings.beginGroup("display");
    const QString aspectRatioModeSetting = (aspectRatioMode == Qt::KeepAspectRatio) ? "KeepAspectRatio" : "KeepAspectRatioByExpanding";
    _settings.setValue("aspectRatioMode", QVariant::fromValue(aspectRatioModeSetting));
    _settings.endGroup();
}

QString BigRingSettings::clientId()
{
    if (!_settings.contains("clientId")) {
        const QUuid clientId = QUuid::createUuid();
        const QString asString = clientId.toRfc4122().toHex();
        _settings.setValue("clientId", QVariant::fromValue(asString));
    }
    return _settings.value("clientId").toString();
}
