#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QList>
#include <QMap>
#include <QString>

class ProfileEntry
{
public:
	explicit ProfileEntry(float distance, float slope);
	explicit ProfileEntry();

	float distance() const { return _distance; }
	float slope() const { return _slope; }

private:
	float _distance;
	float _slope;
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
						   QList<DistanceMappingEntry> distanceMappings, QMap<float,ProfileEntry> profile);
	explicit RealLiveVideo();

	bool isValid() const { return !_name.isEmpty(); }
	const QString name() const { return _name; }
	const VideoInformation& videoInformation() const { return _videoInformation; }
	const QList<Course>& courses() const { return _courses; }

	quint32 frameForDistance(const float distance) const;
	float slopeForDistance(const float distance) const;

	static bool compareByName(const RealLiveVideo& rlv1, const RealLiveVideo& rlv2);
private:
	QString _name;
	VideoInformation _videoInformation;
	QList<Course> _courses;
	QMap<float, DistanceMappingEntry> _distanceMappings;
	QMap<float, ProfileEntry> _profile;
};
typedef QList<RealLiveVideo> RealLiveVideoList;

#endif // REALLIVEVIDEO_H
