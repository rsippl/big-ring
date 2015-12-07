#ifndef RIDEFILE_H
#define RIDEFILE_H

#include <QtCore/QDateTime>
#include <QtCore/QJsonObject>
#include <QtCore/QTime>
#include <map>

/**
 * A record of a complete ride.
 */
class RideFile
{
public:
    /**
     * A Sample from the Ride
     */
    struct Sample {
        QTime time;

        float altitude;
        int cadence;
        float distance;
        int heartRate;
        int power;
        float speed;

        QString toString() const;
        QJsonObject toJson() const;
        static Sample fromJson(const QJsonObject &json);
    };
    typedef std::map<QTime,Sample> SampleMap;

    explicit RideFile();
    explicit RideFile(const QDateTime &startTime, const QString &rlvName, const QString &courseName, const SampleMap &&samples);
    explicit RideFile(const QDateTime &startTime, const QString &rlvName, const QString &courseName);
    static RideFile fromJsonDocument(const QJsonObject &json);

    /**
     * Start time of the ride.
     */
    const QDateTime &startTime() const;
    const QString &rlvName() const;
    const QString &courseName() const;
    const SampleMap &samples() const;

    void addSample(const Sample &sample);

    const SampleMap::const_iterator sampleIteratorForTime(const QTime &time) const;
    const Sample &sampleForTime(const QTime &time) const;
    const Sample &sampleForDistance(const float distance) const;

    bool isValid() const;
    QString toString() const;
    QJsonObject toJson() const;
private:
    const QDateTime _startTime;
    const QString _rlvName;
    const QString _courseName;
    SampleMap _samples;
};

#endif // RIDEFILE_H
