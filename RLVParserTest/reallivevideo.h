#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QString>

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
    explicit RealLiveVideo(const QString& name, const VideoInformation& videoInformation);
    explicit RealLiveVideo();

    bool isValid() const { return !_name.isEmpty(); }
    const QString name() const { return _name; }
    const VideoInformation& videoInformation() const { return _videoInformation; }
private:
    QString _name;
    VideoInformation _videoInformation;
};

#endif // REALLIVEVIDEO_H
