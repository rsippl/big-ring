#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QString>

class VideoInformation
{
public:
	explicit VideoInformation(const QString& videoFilename, float frameRate);

	const QString videoFilename;
	const float frameRate;
};

class RealLiveVideo
{
public:
	explicit RealLiveVideo(const VideoInformation& videoInformation);

	const VideoInformation videoInformation;
};

#endif // REALLIVEVIDEO_H
