#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QString>

class VideoInformation
{
public:
	explicit VideoInformation(const QString& videoFilename, float frameRate);
    explicit VideoInformation();
    VideoInformation operator=(const VideoInformation& other);

	const QString videoFilename;
	const float frameRate;
};

class RealLiveVideo
{
public:
    explicit RealLiveVideo(const VideoInformation& videoInformation);
    explicit RealLiveVideo();
    RealLiveVideo operator=(const RealLiveVideo& other);

	const VideoInformation videoInformation;
};

#endif // REALLIVEVIDEO_H
