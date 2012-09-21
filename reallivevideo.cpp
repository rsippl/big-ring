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
							 QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings, QMap<float, ProfileEntry> profile):
	_name(name), _videoInformation(videoInformation), _courses(courses), _profile(profile)
{
	float currentDistance = 0.0f;

	quint32 lastFrameNumber = 0;
//	float lastMetersPerFrame = 0.0f;
	QListIterator<DistanceMappingEntry> it(distanceMappings);
	while(it.hasNext()) {
		const DistanceMappingEntry& entry = it.next();
		quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
		currentDistance += nrFrames * entry.metersPerFrame();

		_distanceMappings[currentDistance] = entry;

		lastFrameNumber = entry.frameNumber();
//		lastMetersPerFrame = entry.metersPerFrame();
	}

}

RealLiveVideo::RealLiveVideo() {}

quint32 RealLiveVideo::frameForDistance(const float distance) const
{
	qint32 frameNumber = 0;
	float metersPerFrame = 0.0f;
	float keyDistance = 0.0f;
	float lastDistance = 0.0f;

	QMapIterator<float, DistanceMappingEntry> it(_distanceMappings);
	while(it.hasNext()) {
		it.next();

		keyDistance = it.key();
		if (keyDistance > distance)
			break;

		frameNumber = it.value().frameNumber();
		metersPerFrame = it.value().metersPerFrame();
		lastDistance = keyDistance;
	}

	float afterKeyDistance = distance - lastDistance;

	return frameNumber + (afterKeyDistance / metersPerFrame);
}

float RealLiveVideo::slopeForDistance(const float distance) const
{
	float lastSlope = 0.0f;
	QMapIterator<float, ProfileEntry> it(_profile);
	while(it.hasNext()) {
		it.next();

		float currentDistance = it.key();
		ProfileEntry entry = it.value();
		qDebug() << "distance: " << distance << " entry.distance() = " << currentDistance << " slope = " << entry.slope();
		if (currentDistance > distance)
			break;
		lastSlope = entry.slope();
	}
	return lastSlope;
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


