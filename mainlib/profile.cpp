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

#include "profile.h"

#include <utility>
#include <QtCore/QtDebug>

ProfileEntry::ProfileEntry(float distance, float slope, float altitude):
    _distance(distance),_altitude(altitude), _slope(slope)
{
}

ProfileEntry::ProfileEntry():
    _distance(.0f), _slope(.0f)
{
}

bool ProfileEntry::operator ==(const ProfileEntry &other) const
{
    return ((other._distance == _distance) && (other._slope == _slope));
}

Profile::Profile(ProfileType type, float startAltitude, const std::vector<ProfileEntry> &&entries):
    _type(type),
    _startAltitude(startAltitude),
    _entries(entries)
{
    // empty
}

Profile::Profile(): _type(ProfileType::SLOPE), _startAltitude(0.0f)
{
    // empty
}

float Profile::slopeForDistance(float distance) const
{
    return entryForDistance(distance).slope();
}

float Profile::altitudeForDistance(float distance) const
{
    const ProfileEntry& entry = entryForDistance(distance);

    return _startAltitude + entry.altitude() + entry.slope() * 0.01 * (distance - entry.distance());
}

float Profile::minimumAltitude() const
{
    return minimumAltitudeForPart(0, totalDistance());
}

float Profile::minimumAltitudeForPart(float start, float end) const
{
    auto startAndEnd = rangeForDistances(start, end);

    return (*std::min_element(startAndEnd.first, startAndEnd.second,
            [](const ProfileEntry &entry1, const ProfileEntry &entry2) {
        return (entry1.altitude() < entry2.altitude());
    })).altitude() + _startAltitude;
}

float Profile::maximumAltitude() const
{
    return maximumAltitudeForPart(0, totalDistance());
}

float Profile::maximumAltitudeForPart(float start, float end) const
{
    const auto startAndEnd = rangeForDistances(start, end);

    const float maxKeyAltitude = (*std::max_element(startAndEnd.first, startAndEnd.second,
            [](const ProfileEntry &entry1, const ProfileEntry &entry2) {
        return (entry1.altitude() < entry2.altitude());
    })).altitude() + _startAltitude;
    const float endAltitude = altitudeForDistance(end);

    return std::max(maxKeyAltitude, endAltitude);
}

const std::pair<std::vector<ProfileEntry>::const_iterator, std::vector<ProfileEntry>::const_iterator> Profile::rangeForDistances(float start, float end) const
{
    auto startIt = std::lower_bound(_entries.begin(), _entries.end(), start, [](const ProfileEntry &entry, float distance2) {
        return entry.distance() < distance2;
    });
    if (startIt != _entries.begin() && (*startIt).distance() > start) {
        --startIt;
    }
    auto endIt = std::lower_bound(_entries.begin(), _entries.end(), end, [](const ProfileEntry &entry, float distance2) {
        return entry.distance() < distance2;
    });
    return std::make_pair(startIt, endIt);
}

const ProfileEntry &Profile::entryForDistance(float distance) const
{
    if (distance < _lastKeyDistance || distance > _nextLastKeyDistance) {
        unsigned int i = (distance > _nextLastKeyDistance && _nextLastKeyDistance > 0) ? _currentProfileEntryIndex + 1: 0;
        for (; i < _entries.size(); ++i) {
            const ProfileEntry &nextEntry = _entries[i];
            if (nextEntry.distance() > distance) {
                break;
            } else {
                _currentProfileEntryIndex = i;
            }
        }

        _lastKeyDistance = _entries[_currentProfileEntryIndex].distance();
        if (_currentProfileEntryIndex + 1 < _entries.size()) {
            _nextLastKeyDistance = _entries[_currentProfileEntryIndex + 1].distance();
        } else {
            _nextLastKeyDistance = 0;
        }
    }
    return _entries[_currentProfileEntryIndex];
}

float Profile::totalDistance() const {
    if (_entries.empty()) {
        return 0;
    }
    return _entries.back().distance();
}

