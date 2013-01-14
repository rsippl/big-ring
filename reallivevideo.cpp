#include "reallivevideo.h"

#include <QtDebug>
#include <QMapIterator>
Course::Course():
	_start(0.0), _end(0.0)
{}

Course::Course(const QString &name, float start, float end):
	_name(name), _start(start), _end(end)
{}

RealLiveVideo::RealLiveVideo(const QString& name, const VideoInformation& videoInformation,
							 QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings, Profile profile):
	_name(name), _videoInformation(videoInformation), _courses(courses), _profile(profile)
{
	float currentDistance = 0.0f;
	float lastMetersPerFrame = 0;

	quint32 lastFrameNumber = distanceMappings[0].frameNumber();
	QListIterator<DistanceMappingEntry> it(distanceMappings);
	while(it.hasNext()) {
		const DistanceMappingEntry& entry = it.next();
		quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
		currentDistance += nrFrames * lastMetersPerFrame;
		_distanceMappings.append(qMakePair(currentDistance, entry));

		lastMetersPerFrame = entry.metersPerFrame();
		lastFrameNumber = entry.frameNumber();
	}
}

RealLiveVideo::RealLiveVideo() {}

float RealLiveVideo::metersPerFrame(const float distance)
{
	const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(distance);
	return entry.second.metersPerFrame();
}

quint32 RealLiveVideo::frameForDistance(const float distance)
{
	const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(distance);
	return entry.second.frameNumber() + (distance - entry.first) / entry.second.metersPerFrame();
}

float RealLiveVideo::slopeForDistance(const float distance)
{
	return _profile.slopeForDistance(distance);
}

float RealLiveVideo::altitudeForDistance(const float distance)
{
	return _profile.altitudeForDistance(distance);
}

float RealLiveVideo::totalDistance() const
{
	float maxDistance = 0.0f;
	foreach(const Course& course, _courses) {
		maxDistance = qMax(course.end(), maxDistance);
	}
	return maxDistance;
}

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
	_videoFilename(videoFilename), _frameRate(frameRate)
{
}

VideoInformation::VideoInformation():
	_frameRate(0.0) {}

bool RealLiveVideo::compareByName(const RealLiveVideo &rlv1, const RealLiveVideo &rlv2)
{
	return rlv1.name().toLower() < rlv2.name().toLower();
}

const QPair<float,DistanceMappingEntry>& RealLiveVideo::findDistanceMappingEntryFor(const float distance)
{
	if (distance > _lastKeyDistance && distance < _nextLastKeyDistance) {
		return _cachedDistanceMapping;
	}

	QPair<float,DistanceMappingEntry> newEntry;
	QListIterator<QPair<float, DistanceMappingEntry> > it(_distanceMappings);
	while(it.hasNext()) {
		newEntry = it.peekNext();

		if (newEntry.first > distance)
			break;
		else
			_cachedDistanceMapping = it.next();
	}
	_lastKeyDistance = _cachedDistanceMapping.first;
	if (it.hasNext())
		_nextLastKeyDistance = it.peekNext().first;
	else
		_nextLastKeyDistance = 0;
	return _cachedDistanceMapping;
}

DistanceMappingEntry::DistanceMappingEntry(quint32 frameNumber, float metersPerFrame):
	_frameNumber(frameNumber), _metersPerFrame(metersPerFrame)
{
}

DistanceMappingEntry::DistanceMappingEntry():
	_frameNumber(0), _metersPerFrame(0.0f)
{
}


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
