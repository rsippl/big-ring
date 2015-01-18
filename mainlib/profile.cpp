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

ProfileEntry::ProfileEntry(float distance, float totalDistance, float slope, float altitude):
	_distance(distance), _totalDistance(totalDistance),_altitude(altitude), _slope(slope)
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


Profile::Profile(ProfileType type, float startAltitude, QList<ProfileEntry>& entries):
	_type(type),
	_startAltitude(startAltitude), _entries(entries), _lastKeyDistance(0), _nextLastKeyDistance(0)
{
}

Profile::Profile(): _type(SLOPE), _startAltitude(0.0f)
{
}

float Profile::slopeForDistance(double distance)
{
	return entryForDistance(distance).slope();
}

float Profile::altitudeForDistance(double distance)
{
	ProfileEntry& entry = entryForDistance(distance);

	return _startAltitude + entry.altitude() + entry.slope() * 0.01 * (distance - entry.totalDistance());
}

const QList<ProfileEntry> &Profile::entries() const
{
	return _entries;
}

ProfileEntry &Profile::entryForDistance(double distance)
{
	if (distance < _lastKeyDistance || distance > _nextLastKeyDistance) {
		QListIterator<ProfileEntry> it(_entries);
		while(it.hasNext()) {
			const ProfileEntry& newEntry = it.peekNext();

			if (newEntry.totalDistance() > distance)
				break;
			else {
				_cachedProfileEntry = it.next();
			}
		}

		_lastKeyDistance = _cachedProfileEntry.totalDistance();
		if (it.hasNext())
			_nextLastKeyDistance = it.peekNext().totalDistance();
		else
			_nextLastKeyDistance = 0;
	}
	return _cachedProfileEntry;
}

float Profile::totalDistance() const {
    if (_entries.isEmpty()) {
        return 0;
    }
    return _entries.last().totalDistance();
}

