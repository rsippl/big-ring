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

enum ProfileType {
	POWER, SLOPE, HEART_RATE
};

class Profile
{
public:
	explicit Profile(ProfileType type, float startAltitude, QList<ProfileEntry> &entries);
	explicit Profile();

	ProfileType type() const { return _type; }
	float startAltitude() const { return _startAltitude; }
	//! get the slope for a particular distance
	float slopeForDistance(double distance);

	//! get the altitude for a particular distance. The profile always starts at altitude 0.0f
	float altitudeForDistance(double distance);
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
