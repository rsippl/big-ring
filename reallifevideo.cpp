#include "reallifevideo.h"

#include <QtDebug>
#include <QMapIterator>
Course::Course():
	_start(0.0), _end(0.0)
{}

Course::Course(const QString &name, float start, float end):
	_name(name), _start(start), _end(end)
{}

RealLifeVideo::RealLifeVideo(const QString& name, const VideoInformation& videoInformation,
							 QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings, Profile profile):
	_name(name), _videoInformation(videoInformation), _courses(courses), _profile(profile),
	_lastKeyDistance(0), _nextLastKeyDistance(0)
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

RealLifeVideo::RealLifeVideo() {}

float RealLifeVideo::metersPerFrame(const float distance)
{
	const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(distance);
	return entry.second.metersPerFrame();
}

quint32 RealLifeVideo::frameForDistance(const float distance)
{
	const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(distance);
	return entry.second.frameNumber() + (distance - entry.first) / entry.second.metersPerFrame();
}

float RealLifeVideo::slopeForDistance(const float distance)
{
	return _profile.slopeForDistance(distance);
}

float RealLifeVideo::altitudeForDistance(const float distance)
{
	return _profile.altitudeForDistance(distance);
}

float RealLifeVideo::totalDistance() const
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

bool RealLifeVideo::compareByName(const RealLifeVideo &rlv1, const RealLifeVideo &rlv2)
{
	return rlv1.name().toLower() < rlv2.name().toLower();
}

const QPair<float,DistanceMappingEntry>& RealLifeVideo::findDistanceMappingEntryFor(const float distance)
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


