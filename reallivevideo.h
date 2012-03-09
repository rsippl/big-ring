#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QList>
#include <QString>

class Course
{
public:
    explicit Course(const QString& name, float start, float end);
    explicit Course();

    const QString& name() const { return _name; }
    float start() const { return _start; }
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
    explicit RealLiveVideo(const QString& name, const VideoInformation& videoInformation, QList<Course>& courses);
    explicit RealLiveVideo();

    bool isValid() const { return !_name.isEmpty(); }
    const QString name() const { return _name; }
    const VideoInformation& videoInformation() const { return _videoInformation; }
    const QList<Course>& courses() const { return _courses; }

    static bool compareByName(const RealLiveVideo& rlv1, const RealLiveVideo& rlv2);
private:
    QString _name;
    VideoInformation _videoInformation;
    QList<Course> _courses;
};
typedef QList<RealLiveVideo> RealLiveVideoList;

#endif // REALLIVEVIDEO_H
