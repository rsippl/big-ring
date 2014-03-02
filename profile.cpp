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

