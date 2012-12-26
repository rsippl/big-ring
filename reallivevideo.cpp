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
	quint32 lastFrameNumber = 0;
	QListIterator<DistanceMappingEntry> it(distanceMappings);
	qDebug() << name;
	while(it.hasNext()) {
		const DistanceMappingEntry& entry = it.next();
		quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
		currentDistance += nrFrames * lastMetersPerFrame;
		_distanceMappings[currentDistance] = entry;

		lastMetersPerFrame = entry.metersPerFrame();
		lastFrameNumber = entry.frameNumber();
	}
}

RealLiveVideo::RealLiveVideo() {}

float RealLiveVideo::metersPerFrame(const float distance)
{
	float keyDistance = findDistanceMappingEntryFor(distance);
	const DistanceMappingEntry entry = _distanceMappings[keyDistance];
	return entry.metersPerFrame();
}

quint32 RealLiveVideo::frameForDistance(const float distance)
{
	float keyDistance = findDistanceMappingEntryFor(distance);
	const DistanceMappingEntry entry = _distanceMappings[keyDistance];

//	qDebug() << "entry.frameNumber" << entry.frameNumber() << "entry.metersPerFrame" << entry.metersPerFrame()
//	 << "distance" << distance << "keyDistance =" << keyDistance
//	 << "delta" << (distance - keyDistance) << "frame:" <<  (distance - keyDistance) / entry.metersPerFrame();


	return entry.frameNumber() + (distance - keyDistance) / entry.metersPerFrame();
}

float RealLiveVideo::slopeForDistance(const float distance) const
{
	return _profile.slopeForDistance(distance);
}

float RealLiveVideo::altitudeForDistance(const float distance) const
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

float RealLiveVideo::findDistanceMappingEntryFor(const float distance)
{
	if (distance > _lastKeyDistance && distance < _nextLastKeyDistance) {
		return _lastKeyDistance;
	}
	float lastDistance = 0.0f;
	QMapIterator<float, DistanceMappingEntry> it(_distanceMappings);
	while(it.hasNext()) {
		it.next();

		if (it.key() > distance)
			break;
		lastDistance = it.key();
	}
	_lastKeyDistance = lastDistance;
	if (it.hasNext())
		_nextLastKeyDistance = it.peekNext().key();
	else
		_nextLastKeyDistance = 0;
	return lastDistance;
}

DistanceMappingEntry::DistanceMappingEntry(quint32 frameNumber, float metersPerFrame):
	_frameNumber(frameNumber), _metersPerFrame(metersPerFrame)
{
}

DistanceMappingEntry::DistanceMappingEntry():
	_frameNumber(0), _metersPerFrame(0.0f)
{
}


ProfileEntry::ProfileEntry(float distance, float slope):
	_distance(distance), _slope(slope)
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


Profile::Profile(float startAltitude, QMap<float, ProfileEntry> &entries):
	_startAltitude(startAltitude), _entries(entries)
{
}

Profile::Profile():_startAltitude(0.0f)
{
}

float Profile::slopeForDistance(double distance) const
{
	float lastSlope = 0.0f;
	QMapIterator<float, ProfileEntry> it(_entries);
	while(it.hasNext()) {
		it.next();

		float currentDistance = it.key();
		ProfileEntry entry = it.value();
		if (currentDistance > distance)
			break;
		lastSlope = entry.slope();
	}
	return lastSlope;
}

float Profile::altitudeForDistance(double distance) const
{
	float lastDistance = 0.0f;
	float altitude = _startAltitude;
	float lastSlope = 0.0f;
	QMapIterator<float, ProfileEntry> it(_entries);
	while(it.hasNext()) {
		it.next();

		float currentDistance = it.key();
		ProfileEntry entry = it.value();
		if (currentDistance > distance)
			break;

		altitude += (lastSlope / 100.0f) * (currentDistance - lastDistance);

		lastDistance = currentDistance;
		lastSlope = entry.slope();
	}
	altitude += (lastSlope / 100.0f) * (distance - lastDistance);
	return altitude;
}
