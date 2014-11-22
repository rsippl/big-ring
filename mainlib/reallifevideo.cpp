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
	_lastKeyDistance(0), _nextLastKeyDistance(0), _videoCorrectionFactor(1.0)
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
	float correctedDistance = distance * _videoCorrectionFactor;
	const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(correctedDistance);
	return entry.second.frameNumber() + (correctedDistance - entry.first) / entry.second.metersPerFrame();
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


void RealLifeVideo::setDuration(quint64 duration)
{
	quint64 totalNrOfFrames = duration * (_videoInformation.frameRate() / 1000000);
	calculateVideoCorrectionFactor(totalNrOfFrames);
    qDebug() << "correction factor" << _videoCorrectionFactor;
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

/*!
 * \brief Determine the ratio between the length of the video and the
 * length of the profile. Store this ratio in the field _videoCorrectionFactor.
 * This ratio is used whenever we need to determine the exact frame for a difference.
 *
 * There are still some videos where this does not work correctly, like IT_GiroMortirolo,
 * but most videos seem to work ok.
 *
 * \param totalNrOfFrames the total number of frames in the video.
 */
void RealLifeVideo::calculateVideoCorrectionFactor(quint64 totalNrOfFrames)
{
	// some rlvs, mostly the old ones like the old MajorcaTour have only two
	// entries in the distancemappings list. For these rlvs, it seems to work
	// better to just use a _videoCorrectionFactor of 1.0.
	if (_distanceMappings.size() < 3) {
		_videoCorrectionFactor = 1;
	} else {
		auto lastDistanceMapping = _distanceMappings.last();
		quint64 framesInLastEntry;
		if (totalNrOfFrames > lastDistanceMapping.second.frameNumber()) {
			framesInLastEntry = totalNrOfFrames - lastDistanceMapping.second.frameNumber();
		} else {
			framesInLastEntry = 0u;
		}
		float videoDistance = lastDistanceMapping.first + framesInLastEntry * lastDistanceMapping.second.metersPerFrame();
		_videoCorrectionFactor = videoDistance / _profile.totalDistance();
	}
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


