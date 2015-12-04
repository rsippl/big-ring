/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "reallifevideo.h"

#include "distancemappingentry.h"
#include "videoinformation.h"

#include <QtDebug>
#include <QMapIterator>

namespace
{
std::map<RealLifeVideoFileType,const QString> REAL_LIFE_VIDEO_TYPE_NAMES =
{{RealLifeVideoFileType::GPX, "GPX"},
 {RealLifeVideoFileType::TACX, "TACX"},
 {RealLifeVideoFileType::VIRTUAL_TRAINING, "Cycleops Virtual Training"}};
const QString REAL_LIFE_VIDEO_TYPE_NAMES_UNKNOWN_TYPE;
}

const QString &realLifeVideoFileTypeName(RealLifeVideoFileType type) {
    auto it = REAL_LIFE_VIDEO_TYPE_NAMES.find(type);
    if (it == REAL_LIFE_VIDEO_TYPE_NAMES.end()) {
        return REAL_LIFE_VIDEO_TYPE_NAMES_UNKNOWN_TYPE;
    }
    return (*it).second;
}

class RealLifeVideoData
{
public:
    RealLifeVideoData()  {}

    ~RealLifeVideoData() {}

    QString _name;
    RealLifeVideoFileType _fileType;
    Profile _profile;
    std::vector<DistanceMappingEntry> _distanceMappings;
    VideoInformation _videoInformation;
    std::vector<Course> _courses;
    std::vector<InformationBox> _informationBoxes;
    float _videoCorrectionFactor;
};

Course::Course(const QString &name, const Type type, float start, float end):
    _name(name), _type(type), _start(start), _end(end)
{}

Course::Course(const QString &name, float start, float end):
    Course(name, Type::Normal, start, end)
{
    // empty
}

Course::Course(): Course("", Type::Invalid, 0, 0)
{
    // empty
}


Course::Course(float start, float end):
    Course(QObject::tr("Unfinished Run"), Type::Unfinished, start, end)
{
    // empty
}

RealLifeVideo::RealLifeVideo(const QString& name, RealLifeVideoFileType fileType, const VideoInformation& videoInformation,
                             const QList<Course>& courses, const QList<DistanceMappingEntry>& distanceMappings, Profile profile):
    RealLifeVideo(name, fileType, videoInformation, std::move(std::vector<Course>(courses.begin(), courses.end())),
                  std::move(std::vector<DistanceMappingEntry>(distanceMappings.begin(), distanceMappings.end())), profile)
{
    // empty
}

RealLifeVideo::RealLifeVideo(const QString &name, RealLifeVideoFileType fileType, const VideoInformation &videoInformation,
                             const std::vector<Course> &&courses,
                             const std::vector<DistanceMappingEntry> &&distanceMappings,
                             Profile& profile, const std::vector<InformationBox> &&informationBoxes):
    _d(new RealLifeVideoData)
{
    _d->_name = name;
    _d->_fileType = fileType;
    _d->_profile = profile;
    _d->_videoInformation = videoInformation;
    _d->_courses = courses;
    _d->_videoCorrectionFactor = 1.0;
    _d->_distanceMappings = distanceMappings;
    _d->_informationBoxes = informationBoxes;
}

RealLifeVideo::RealLifeVideo(const RealLifeVideo &other):
    _d(other._d), _lastKeyDistance(other._lastKeyDistance), _nextLastKeyDistance(other._nextLastKeyDistance)
{

}

RealLifeVideo::RealLifeVideo():
    _d(new RealLifeVideoData)
{

}

bool RealLifeVideo::isValid() const
{
    return (!_d->_name.isEmpty() && !_d->_videoInformation.videoFilename().isEmpty());
}

RealLifeVideoFileType RealLifeVideo::fileType() const
{
    return _d->_fileType;
}

ProfileType RealLifeVideo::type() const
{
    return _d->_profile.type();
}

const Profile &RealLifeVideo::profile() const
{
    return _d->_profile;
}

const QString RealLifeVideo::name() const
{
    return _d->_name;
}

const QString &RealLifeVideo::videoFilename() const
{
    return _d->_videoInformation.videoFilename();
}

float RealLifeVideo::videoFrameRate() const
{
    return _d->_videoInformation.frameRate();
}

const std::vector<Course> &RealLifeVideo::courses() const
{
    return _d->_courses;
}

const std::vector<DistanceMappingEntry> &RealLifeVideo::distanceMappings() const
{
    return _d->_distanceMappings;
}

const std::vector<InformationBox> &RealLifeVideo::informationBoxes() const
{
    return _d->_informationBoxes;
}

void RealLifeVideo::setUnfinishedRun(float distance)
{
    if (!_d->_courses.empty() && _d->_courses.back().type() == Course::Type::Unfinished) {
        _d->_courses.pop_back();
    }
    Course unfinishedCourse(distance, totalDistance());
    _d->_courses.push_back(unfinishedCourse);
}

void RealLifeVideo::addStartPoint(float distance, const QString &name)
{
    addCustomCourse(distance, totalDistance(), name);
}

void RealLifeVideo::addCustomCourse(float startDistance, float endDistance, const QString &name)
{
    Course customCourse(name, Course::Type::Custom, startDistance, endDistance);
    _d->_courses.push_back(customCourse);
}

void RealLifeVideo::printDistanceMapping()
{
    int i = 0;
    for(auto distanceMapping: _d->_distanceMappings) {
        qDebug() << i++ << distanceMapping.frameNumber() << distanceMapping.distance();
    }
    qDebug() << "framerate" << _d->_videoInformation.frameRate();
}


float RealLifeVideo::metersPerFrame(const float distance) const
{
    const DistanceMappingEntry& entry = findDistanceMappingEntryFor(distance);
    return entry.metersPerFrame();
}

quint32 RealLifeVideo::frameForDistance(const float distance) const
{
    float correctedDistance = distance * _d->_videoCorrectionFactor;
    const DistanceMappingEntry& entry = findDistanceMappingEntryFor(correctedDistance);
    return entry.frameNumber() + (correctedDistance - entry.distance()) / entry.metersPerFrame();
}

float RealLifeVideo::slopeForDistance(const float distance) const
{
    return _d->_profile.slopeForDistance(distance);
}

float RealLifeVideo::altitudeForDistance(const float distance) const
{
    return _d->_profile.altitudeForDistance(distance);
}

const InformationBox RealLifeVideo::informationBoxForDistance(const float distance) const
{
    if (fileType() == RealLifeVideoFileType::TACX) {
        return informationBoxForDistanceTacx(distance);
    }

    // default implementation.
    InformationBox informationBoxForDistance;
    for (const InformationBox &informationBox: _d->_informationBoxes) {
        if (informationBox.distance() > distance) {
            break;
        }
        informationBoxForDistance = informationBox;
    }
    return informationBoxForDistance;
}

float RealLifeVideo::totalDistance() const
{
    return _d->_profile.totalDistance();
}


void RealLifeVideo::setNumberOfFrames(quint64 numberOfFrames)
{
    if (_d->_fileType == RealLifeVideoFileType::TACX) {
        calculateVideoCorrectionFactor(numberOfFrames);
        qDebug() << "correction factor" << _d->_videoCorrectionFactor;
    } else {
        qDebug() << "no correction factor needed.";
    }
}

bool RealLifeVideo::operator==(const RealLifeVideo &other) const
{
    return isValid() && other.isValid() && _d->_name == other._d->_name
            && _d->_fileType == other._d->_fileType;
}

bool RealLifeVideo::compareByName(const RealLifeVideo &rlv1, const RealLifeVideo &rlv2)
{
    return rlv1.name().toLower() < rlv2.name().toLower();
}

/*!
 * \brief Determine the ratio between the length of the video and the
 * length of the profile. Store this ratio in the field _videoCorrectionFactor.
 * This ratio is used whenever we need to determine the exact frame for a difference.
 *
 * There are still some videos where this does not work correctly, like IT_GiroMortirolo,
 * but most videos seem to work ok.
 *
 * \param totalNrOfFrames the total number of frames in the video.
 */
void RealLifeVideo::calculateVideoCorrectionFactor(quint64 totalNrOfFrames)
{
    // some rlvs, mostly the old ones like the old MajorcaTour have only two
    // entries in the distancemappings list. For these rlvs, it seems to work
    // better to just use a _videoCorrectionFactor of 1.0.
    if (_d->_distanceMappings.size() < 3) {
        _d->_videoCorrectionFactor = 1;
    } else {
        const auto &lastDistanceMapping = _d->_distanceMappings.back();
        quint64 framesInLastEntry;
        if (totalNrOfFrames > lastDistanceMapping.frameNumber()) {
            framesInLastEntry = totalNrOfFrames - lastDistanceMapping.frameNumber();
        } else {
            framesInLastEntry = 0u;
        }
        float videoDistance = lastDistanceMapping.distance() + framesInLastEntry * lastDistanceMapping.metersPerFrame();
        _d->_videoCorrectionFactor = videoDistance / _d->_profile.totalDistance();
    }
}

const DistanceMappingEntry &RealLifeVideo::findDistanceMappingEntryFor(const float distance) const
{
    if (distance < _lastKeyDistance || distance > _nextLastKeyDistance) {
        unsigned i = (distance > _nextLastKeyDistance) ? _currentDistanceMappingIndex + 1: 0;
        for (; i < _d->_distanceMappings.size(); ++i) {
            const DistanceMappingEntry &nextEntry = _d->_distanceMappings[i];
            if (nextEntry.distance() > distance) {
                break;
            } else {
                _currentDistanceMappingIndex = i;
            }
        }

        _lastKeyDistance = _d->_distanceMappings[_currentDistanceMappingIndex].distance();
        if (_currentDistanceMappingIndex + 1 < _d->_distanceMappings.size()) {
            _nextLastKeyDistance = _d->_distanceMappings[_currentDistanceMappingIndex + 1].distance();
        } else {
            _nextLastKeyDistance = 0;
        }
    }
    return _d->_distanceMappings[_currentDistanceMappingIndex];
}

const InformationBox RealLifeVideo::informationBoxForDistanceTacx(const float distance) const
{
    quint32 frame = frameForDistance(distance);
    InformationBox informationBoxForDistance;
    for (const InformationBox &informationBox: _d->_informationBoxes) {
        if (informationBox.frameNumber() > frame) {
            break;
        }
        informationBoxForDistance = informationBox;
    }
    return informationBoxForDistance;
}

bool operator==(const InformationBox &lhs, const InformationBox &rhs)
{
    return lhs.frameNumber() == rhs.frameNumber() && qFuzzyCompare(lhs.distance(), rhs.distance());
}
