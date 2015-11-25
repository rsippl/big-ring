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
    _currentProfileEntry = _entries.begin();
}

Profile::Profile(const Profile &other):
    _type(other._type), _startAltitude(other._startAltitude)
{
    _entries = other._entries;
    _currentProfileEntry = _entries.begin();
}

Profile::Profile():
    Profile(ProfileType::SLOPE, 0.0f, std::vector<ProfileEntry>())
{
    // empty
}

Profile &Profile::operator=(const Profile &other)
{
    _type = other._type;
    _startAltitude = other._startAltitude;
    _entries = other._entries;
    _currentProfileEntry = _entries.begin();
    return *this;
}

float Profile::slopeForDistance(float distance) const
{
    return entryIteratorForDistance(distance)->slope();
}

float Profile::altitudeForDistance(float distance) const
{
    const auto& it = entryIteratorForDistance(distance);

    return _startAltitude + it->altitude() + it->slope() * 0.01 * (distance - it->distance());
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

const std::pair<const Profile::ProfileEntryVectorIt, const Profile::ProfileEntryVectorIt> Profile::rangeForDistances(float start, float end) const
{
    auto startIt = std::lower_bound(_entries.begin(), _entries.end(), start, [](const ProfileEntry &entry, float distance2) {
        return entry.distance() < distance2;
    });
    if (startIt != _entries.begin() && (*startIt).distance() > start) {
        --startIt;
    }
    auto endIt = std::lower_bound(startIt, _entries.end(), end, [](const ProfileEntry &entry, float distance2) {
        return entry.distance() < distance2;
    });
    return std::make_pair(startIt, endIt);
}

const Profile::ProfileEntryVectorIt Profile::entryIteratorForDistance(const float distance) const
{
    const bool atLastEntry = _currentProfileEntry + 1 == _entries.end();
    const bool isDistanceSmallerThenCurrent = distance < _currentProfileEntry->distance();
    const bool isDistanceBiggerThenCurrent = !atLastEntry && distance > (_currentProfileEntry + 1)->distance();

    // if the distance we're looking for is smaller than the current entry, or bigger than the
    // start of the next entry, then we need to search which entry to use. If not, we can
    // simply use the current entry.
    if (isDistanceSmallerThenCurrent || (!atLastEntry && isDistanceBiggerThenCurrent)) {
        auto startIterator = (isDistanceBiggerThenCurrent) ? _currentProfileEntry + 1 : _entries.begin();
        auto it = std::lower_bound(startIterator, _entries.end(), distance, [](const ProfileEntry &entry, float distance) {
            return entry.distance() < distance;
        });
        // lower bound gives us the first entry that is not smaller then distance, so we need to
        // go back one step, unless we're already at the beginning.
        if (it != _entries.begin() && (it == _entries.end() || it->distance() > distance) ) {
            it--;
        }
        _currentProfileEntry = it;
    }
    return _currentProfileEntry;
}

float Profile::totalDistance() const {
    if (_entries.empty()) {
        return 0;
    }
    return _entries.back().distance();
}

float Profile::startAltitude() const
{
    return _startAltitude;
}

const std::vector<ProfileEntry> &Profile::entries() const
{
    return _entries;
}

