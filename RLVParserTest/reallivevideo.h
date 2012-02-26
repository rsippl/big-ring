#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QString>

class VideoInformation
{
public:
	VideoInformation(const QString& videoFilename, float frameRate);

	const QString videoFilename;
	const float frameRate;
};

class RealLiveVideo
{
public:
	RealLiveVideo(const VideoInformation& videoInformation);

	const VideoInformation videoInformation;
};

#endif // REALLIVEVIDEO_H
