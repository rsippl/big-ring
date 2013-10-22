#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

#include "profile.h"

class DistanceMappingEntry
{
public:
	explicit DistanceMappingEntry(quint32 frameNumber, float metersPerFrame);
	explicit DistanceMappingEntry();

	quint32 frameNumber() const { return _frameNumber; }
	float metersPerFrame() const { return _metersPerFrame; }

private:
	quint32 _frameNumber;
	float _metersPerFrame;
};

class Course
{
public:
	explicit Course(const QString& name, float start, float end);
	explicit Course();

	const QString& name() const { return _name; }
	/*! start distance of course in  meters */
	float start() const { return _start; }
	/*! end distance of course in meters */
	float end() const { return _end; }

private:
	QString _name;
	float _start;
	float _end;
};

class VideoInformation
{
public:
	explicit VideoInformation(const QString& videoFilename, float frameRate);
	explicit VideoInformation();

	const QString& videoFilename() const { return _videoFilename; }
	float frameRate() const { return _frameRate; }

private:
	QString _videoFilename;
	float _frameRate;
};

class RealLifeVideo
{
public:
	explicit RealLifeVideo(const QString& name, const VideoInformation& videoInformation, QList<Course>& courses,
						   QList<DistanceMappingEntry> distanceMappings, Profile profile);
	explicit RealLifeVideo();

	bool isValid() const { return (!_name.isEmpty() && !_videoInformation.videoFilename().isEmpty()); }
	ProfileType type() const { return _profile.type(); }
	Profile& profile() { return _profile; }
	const QString name() const { return _name; }
	const VideoInformation& videoInformation() const { return _videoInformation; }
	const QList<Course>& courses() const { return _courses; }

	/** Get the number or frames per meter for a certain distance */
	float metersPerFrame(const float distance);
	/** Get the exact frame for a distance. */
	quint32 frameForDistance(const float distance);
	/** Get the slope for a distance */
    float slopeForDistance(const float distance);
	//! Get the altitude for a distance */
    float altitudeForDistance(const float distance);
	/** Total distance */
	float totalDistance() const;

	static bool compareByName(const RealLifeVideo& rlv1, const RealLifeVideo& rlv2);
private:
	const QPair<float, DistanceMappingEntry> &findDistanceMappingEntryFor(const float distance);

	QString _name;
    VideoInformation _videoInformation;
	QList<Course> _courses;
	QList<QPair<float,DistanceMappingEntry> > _distanceMappings;
	Profile _profile;
	QPair<float,DistanceMappingEntry> _cachedDistanceMapping;
	float _lastKeyDistance;
	float _nextLastKeyDistance;
};
typedef QList<RealLifeVideo> RealLifeVideoList;

#endif // REALLIVEVIDEO_H
