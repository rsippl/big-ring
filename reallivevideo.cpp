#include "reallivevideo.h"

#include <QtDebug>

Course::Course():
	_start(0.0), _end(0.0)
{}

Course::Course(const QString &name, float start, float end):
	_name(name), _start(start), _end(end)
{}

RealLiveVideo::RealLiveVideo(const QString& name, const VideoInformation& videoInformation,
							 QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings):
	_name(name), _videoInformation(videoInformation), _courses(courses), _distanceMappings(distanceMappings)
{
}

RealLiveVideo::RealLiveVideo() {}

quint32 RealLiveVideo::frameForDistance(const float distance) const
{
	foreach (DistanceMappingEntry entry, _distanceMappings) {
		qDebug() << entry.frameNumber() << ", " << entry.metersPerFrame();
	}
	// only works for very old rlvs!
	return distance / _distanceMappings[0].metersPerFrame();
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
