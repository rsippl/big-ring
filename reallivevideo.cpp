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
							 QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings):
	_name(name), _videoInformation(videoInformation), _courses(courses)
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
	qDebug() << "**************************";
	qDebug();
	foreach (float distance, _distanceMappings.keys()) {
		qDebug() << distance << ": " <<  _distanceMappings[distance].frameNumber() << ", " << _distanceMappings[distance].metersPerFrame();
	}
	qDebug() << "**************************";
	qDebug();

	qint32 frameNumber = 0;
	float metersPerFrame = 0.0f;
	float keyDistance = 0.0f;
	float lastDistance = 0.0f;

	QMapIterator<float, DistanceMappingEntry> it(_distanceMappings);
	while(it.hasNext()) {
		it.next();

		keyDistance = it.key();
		qDebug() << "key: " << keyDistance << " looking for " << distance;
		if (keyDistance > distance)
			break;

		frameNumber = it.value().frameNumber();
		metersPerFrame = it.value().metersPerFrame();
		lastDistance = keyDistance;
	}

	float afterKeyDistance = distance - lastDistance;

	return frameNumber + (afterKeyDistance / metersPerFrame);
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
