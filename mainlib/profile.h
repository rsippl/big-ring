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

#ifndef PROFILE_H
#define PROFILE_H

#include <QList>

class ProfileEntry
{
public:
    explicit ProfileEntry(float distance, float totalDistance, float slope, float altitude);
    explicit ProfileEntry();

    float distance() const { return _distance; }
    float slope() const { return _slope; }
    float totalDistance() const { return _totalDistance; }
    float altitude() const { return _altitude; }

    bool operator ==(const ProfileEntry& other) const;
private:
    float _distance;
    float _totalDistance;
    float _altitude;
    float _slope;
};

enum class ProfileType {
    POWER, SLOPE, HEART_RATE
};

class Profile
{
public:
    explicit Profile(ProfileType type, float startAltitude, const QList<ProfileEntry> &entries);
    explicit Profile();

    ProfileType type() const { return _type; }
    float startAltitude() const { return _startAltitude; }
    //! get the slope for a particular distance
    float slopeForDistance(double distance);
    //! total distance of the profile
    float totalDistance() const;

    //! get the altitude for a particular distance. The profile always starts at altitude 0.0f
    float altitudeForDistance(double distance);

    const QList<ProfileEntry>& entries() const;
private:
    ProfileEntry& entryForDistance(double distance);


    ProfileType _type;
    float _startAltitude;
    QList<ProfileEntry> _entries;
    ProfileEntry _cachedProfileEntry;
    float _lastKeyDistance;
    float _nextLastKeyDistance;
};

#endif // PROFILE_H
