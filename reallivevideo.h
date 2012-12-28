#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

class ProfileEntry
{
public:
	explicit ProfileEntry(float distance, float slope);
	explicit ProfileEntry();

	float distance() const { return _distance; }
	float slope() const { return _slope; }

	bool operator ==(const ProfileEntry& other) const;
private:
	float _distance;
	float _slope;
};

class Profile
{
public:
	explicit Profile(float startAltitude, QMap<float,ProfileEntry>& entries);
	explicit Profile();

	float startAltitude() const { return _startAltitude; }
	//! get the slope for a particular distance
	float slopeForDistance(double distance) const;

	//! get the altitude for a particular distance. The profile always starts at altitude 0.0f
	float altitudeForDistance(double distance) const;
private:
	float _startAltitude;
	QMap<float,ProfileEntry> _entries;
};

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

class RealLiveVideo
{
public:
	explicit RealLiveVideo(const QString& name, const VideoInformation& videoInformation, QList<Course>& courses,
						   QList<DistanceMappingEntry> distanceMappings, Profile profile);
	explicit RealLiveVideo();

	bool isValid() const { return !_name.isEmpty(); }
	const QString name() const { return _name; }
	const VideoInformation& videoInformation() const { return _videoInformation; }
	const QList<Course>& courses() const { return _courses; }

	/** Get the number or frames per meter for a certain distance */
	float metersPerFrame(const float distance);
	/** Get the exact frame for a distance. */
	quint32 frameForDistance(const float distance);
	/** Get the slope for a distance */
	float slopeForDistance(const float distance) const;
	//! Get the altitude for a distance */
	float altitudeForDistance(const float distance) const;
	/** Total distance */
	float totalDistance() const;

	static bool compareByName(const RealLiveVideo& rlv1, const RealLiveVideo& rlv2);
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
typedef QList<RealLiveVideo> RealLiveVideoList;

#endif // REALLIVEVIDEO_H
