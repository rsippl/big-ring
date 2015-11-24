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
#ifndef BIGRINGSETTINGS_H
#define BIGRINGSETTINGS_H

#include <QtCore/QSettings>

/**
 * Wrapper around QSettings, used for application specific settings.
 * Just create a BigRingSettings object on the stack and load and
 * save settings.
 */
class BigRingSettings
{
public:
    BigRingSettings();

    QString videoFolder() const;
    void setVideoFolder(const QString folder);

    int powerAveragingForDisplayMilliseconds() const;
    void setPowerAveragingForDisplayMilliseconds(const int averagingMilliseconds);
private:
    QSettings _settings;
};

#endif // BIGRINGSETTINGS_H
