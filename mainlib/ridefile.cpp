#include "ridefile.h"

#include <QtCore/QtDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <functional>
#include <utility>

namespace {
const QString ISO_WITH_MILLISECONDS_FORMAT("hh:mm:ss.zzz");
}

RideFile::RideFile():
    RideFile(QDateTime(), "", "")
{
    // empty
}


RideFile::RideFile(const QDateTime &startTime, const QString &rlvName, const QString &courseName):
    RideFile(startTime, rlvName, courseName, std::map<QTime, Sample>())
{
    // empty
}

RideFile::RideFile(const QDateTime &startTime, const QString &rlvName, const QString &courseName, const std::map<QTime, Sample> &&samples): _startTime(startTime), _rlvName(rlvName), _courseName(courseName), _samples(samples)
{
    // empty
}

RideFile RideFile::fromJsonDocument(const QJsonObject &json)
{
    const QString rlvName = json["rlvName"].toString();
    const QString courseName = json["courseName"].toString();
    const QDateTime startTime = QDateTime::fromString(json["startTime"].toString(), Qt::ISODate);

    const QJsonArray sampleArray = json["samples"].toArray();
    std::map<QTime, Sample> samples;
    for (auto value: sampleArray) {
        const Sample sample = Sample::fromJson(value.toObject());
        samples.insert({ sample.time, sample});
    }
    return RideFile(startTime, rlvName, courseName, std::move(samples));
}

const QDateTime &RideFile::startTime() const
{
    return _startTime;
}

const QString &RideFile::rlvName() const
{
    return _rlvName;
}

const QString &RideFile::courseName() const
{
    return _courseName;
}

int RideFile::durationInMilliSeconds() const
{
    if (_samples.empty()) {
        return 0;
    }
    return _samples.rbegin()->first.msecsSinceStartOfDay();
}

float RideFile::totalDistance() const
{
    if (_samples.empty()) {
        return 0;
    }
    return _samples.rbegin()->second.distance;
}

float RideFile::maximumSpeedInMps() const
{
    float max = 0;
    for (const SampleMapElement &sampleElement: _samples) {
        max = std::max(max, sampleElement.second.speed);
    }
    return max;
}

int RideFile::maximumHeartRate() const
{
    int max = 0;
    for (const SampleMapElement &sampleElement: _samples) {
        max = std::max(max, sampleElement.second.heartRate);
    }
    return max;
}

const RideFile::SampleMap &RideFile::samples() const
{
    return _samples;
}

void RideFile::addSample(const RideFile::Sample &sample)
{
    _samples.insert(std::make_pair(sample.time, sample));
}

const RideFile::SampleMap::const_iterator RideFile::sampleIteratorForTime(const QTime &time) const
{
    auto it = _samples.upper_bound(time);

    if (it != _samples.begin()) {
        --it;
    }
    return it;
}

const RideFile::Sample &RideFile::sampleForTime(const QTime &time) const
{
    return sampleIteratorForTime(time)->second;
}

const RideFile::Sample &RideFile::sampleForDistance(const float distance) const
{
    auto it = std::upper_bound(_samples.begin(), _samples.end(), distance, [](float distance, const std::pair<QTime, Sample> &sample) {
        return distance < sample.second.distance;
    });

    if (it != _samples.begin()) {
        --it;
    }
    return it->second;
}

bool RideFile::isValid() const
{
    return !_rlvName.isEmpty() && _startTime.isValid() && !_samples.empty();
}

QString RideFile::toString() const
{
    return QJsonDocument(toJson()).toJson();
}

QJsonObject RideFile::toJson() const
{
    QJsonObject root;
    root["rlvName"] = _rlvName;
    root["courseName"] = _courseName;
    root["startTime"] = _startTime.toString(Qt::ISODate);

    QJsonArray samples;
    for (auto it: _samples) {
        samples.append(it.second.toJson());
    }
    root["samples"] = samples;

    return root;
}


QString RideFile::Sample::toString() const
{
    return time.toString(Qt::ISODate);
}

QJsonObject RideFile::Sample::toJson() const
{
    QJsonObject sampleObject;
    sampleObject["time"] = time.toString(ISO_WITH_MILLISECONDS_FORMAT);
    sampleObject["altitude"] = altitude;
    sampleObject["cadence"] = cadence;
    sampleObject["distance"] = distance;
    sampleObject["heartRate"] = heartRate;
    sampleObject["power"] = power;
    sampleObject["speed"] = speed;

    return sampleObject;
}

RideFile::Sample RideFile::Sample::fromJson(const QJsonObject &json)
{
    return {
        QTime::fromString(json["time"].toString(), ISO_WITH_MILLISECONDS_FORMAT),
                static_cast<float>(json["altitude"].toDouble()),
                json["cadence"].toInt(),
                static_cast<float>(json["distance"].toDouble()),
                json["heartRate"].toInt(),
                json["power"].toInt(),
                static_cast<float>(json["speed"].toDouble())
    };
}
