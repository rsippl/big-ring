#include "reallivevideo.h"

RealLiveVideo::RealLiveVideo(const VideoInformation& videoInformation):
	videoInformation(videoInformation)
{
}

RealLiveVideo::RealLiveVideo() {}

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
	videoFilename(videoFilename), frameRate(frameRate)
{
}

VideoInformation::VideoInformation():
    frameRate(0.0) {}

RealLiveVideo RealLiveVideo::operator =(const RealLiveVideo &other)
{
    return RealLiveVideo(other.videoInformation);
}

VideoInformation VideoInformation::operator =(const VideoInformation &other)
{
    return VideoInformation(other.videoFilename, other.frameRate);
}


