#include "reallivevideo.h"

RealLiveVideo::RealLiveVideo(const VideoInformation& videoInformation):
	videoInformation(videoInformation)
{
}

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
	videoFilename(videoFilename), frameRate(frameRate)
{
}

