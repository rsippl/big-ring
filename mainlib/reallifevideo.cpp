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

#include <QtDebug>
#include <QMapIterator>

class RealLifeVideoData
{
public:
    RealLifeVideoData()  {}

    ~RealLifeVideoData() {}

    QString _name;
    Profile _profile;
    QList<QPair<float,DistanceMappingEntry> > _distanceMappings;
    VideoInformation _videoInformation;
    QList<Course> _courses;
    QPair<float,DistanceMappingEntry> _cachedDistanceMapping;
    float _videoCorrectionFactor;
};

Course::Course(const QString &name, const Type type, float start, float end):
    _name(name), _type(type), _start(start), _end(end)
{}

Course::Course(const QString &name, float start, float end):
    Course(name, Normal, start, end)
{
    // empty
}

Course::Course(): Course("", Invalid, 0, 0)
{
    // empty
}


Course::Course(float start, float end):
    Course(QObject::tr("Unfinished Run"), Unfinished, start, end)
{
    // empty
}

RealLifeVideo::RealLifeVideo(const QString& name, const VideoInformation& videoInformation,
                             QList<Course>& courses, QList<DistanceMappingEntry> distanceMappings, Profile profile):
    _d(new RealLifeVideoData),
    _lastKeyDistance(0), _nextLastKeyDistance(0)
{
    _d->_name = name;
    _d->_profile = profile;
    _d->_videoInformation = videoInformation;
    _d->_courses = courses;
    _d->_videoCorrectionFactor = 1.0;

    float currentDistance = 0.0f;
    float lastMetersPerFrame = 0;

    quint32 lastFrameNumber = distanceMappings[0].frameNumber();
    QListIterator<DistanceMappingEntry> it(distanceMappings);
    QList<QPair<float,DistanceMappingEntry> > distanceMappingsWithDistance;
    while(it.hasNext()) {
        const DistanceMappingEntry& entry = it.next();
        quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
        currentDistance += nrFrames * lastMetersPerFrame;
        distanceMappingsWithDistance.append(qMakePair(currentDistance, entry));

        lastMetersPerFrame = entry.metersPerFrame();
        lastFrameNumber = entry.frameNumber();
    }
    _d->_distanceMappings = distanceMappingsWithDistance;
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

ProfileType RealLifeVideo::type() const
{
    return _d->_profile.type();
}

Profile &RealLifeVideo::profile() const
{
    return _d->_profile;
}

const QString RealLifeVideo::name() const
{
    return _d->_name;
}

const VideoInformation &RealLifeVideo::videoInformation() const
{
    return _d->_videoInformation;
}

const QList<Course> &RealLifeVideo::courses() const
{
    return _d->_courses;
}

void RealLifeVideo::setUnfinishedRun(float distance)
{
    if (!_d->_courses.isEmpty() && _d->_courses.last().type() == Course::Unfinished) {
        _d->_courses.removeLast();
    }
    Course unfinishedCourse(distance, totalDistance());
    _d->_courses.append(unfinishedCourse);
}

void RealLifeVideo::addStartPoint(float distance, const QString &name)
{
    Course customCourse(name, Course::Custom, distance, totalDistance());
    _d->_courses.append(customCourse);
}

void RealLifeVideo::printDistanceMapping()
{
    int i = 0;
    for(auto distanceMapping: _d->_distanceMappings) {
        qDebug() << i++ << distanceMapping.second.frameNumber() << distanceMapping.first;
    }
    qDebug() << "framerate" << _d->_videoInformation.frameRate();
}


float RealLifeVideo::metersPerFrame(const float distance)
{
    const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(distance);
    return entry.second.metersPerFrame();
}

quint32 RealLifeVideo::frameForDistance(const float distance)
{
    float correctedDistance = distance * _d->_videoCorrectionFactor;
    const QPair<float,DistanceMappingEntry>& entry = findDistanceMappingEntryFor(correctedDistance);
    return entry.second.frameNumber() + (correctedDistance - entry.first) / entry.second.metersPerFrame();
}

float RealLifeVideo::slopeForDistance(const float distance)
{
    return _d->_profile.slopeForDistance(distance);
}

float RealLifeVideo::altitudeForDistance(const float distance)
{
    return _d->_profile.altitudeForDistance(distance);
}

float RealLifeVideo::totalDistance() const
{
    float maxDistance = 0.0f;
    foreach(const Course& course, _d->_courses) {
        maxDistance = qMax(course.end(), maxDistance);
    }
    return maxDistance;
}


void RealLifeVideo::setNumberOfFrames(quint64 numberOfFrames)
{
    calculateVideoCorrectionFactor(numberOfFrames);
    qDebug() << "correction factor" << _d->_videoCorrectionFactor;
}

bool RealLifeVideo::operator==(const RealLifeVideo &other) const
{
    return isValid() && other.isValid() && _d->_name == other._d->_name;
}

VideoInformation::VideoInformation(const QString &videoFilename, float frameRate):
    _videoFilename(videoFilename), _frameRate(frameRate)
{
}

VideoInformation::VideoInformation():
    _frameRate(0.0) {}

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
        auto lastDistanceMapping = _d->_distanceMappings.last();
        quint64 framesInLastEntry;
        if (totalNrOfFrames > lastDistanceMapping.second.frameNumber()) {
            framesInLastEntry = totalNrOfFrames - lastDistanceMapping.second.frameNumber();
        } else {
            framesInLastEntry = 0u;
        }
        float videoDistance = lastDistanceMapping.first + framesInLastEntry * lastDistanceMapping.second.metersPerFrame();
        _d->_videoCorrectionFactor = videoDistance / _d->_profile.totalDistance();
    }
}

const QPair<float,DistanceMappingEntry>& RealLifeVideo::findDistanceMappingEntryFor(const float distance)
{
    if (distance > _lastKeyDistance && distance < _nextLastKeyDistance) {
        return _d->_cachedDistanceMapping;
    }

    QPair<float,DistanceMappingEntry> newEntry;
    QListIterator<QPair<float, DistanceMappingEntry> > it(_d->_distanceMappings);
    while(it.hasNext()) {
        newEntry = it.peekNext();

        if (newEntry.first > distance)
            break;
        else
            _d->_cachedDistanceMapping = it.next();
    }
    _lastKeyDistance = _d->_cachedDistanceMapping.first;
    if (it.hasNext())
        _nextLastKeyDistance = it.peekNext().first;
    else
        _nextLastKeyDistance = 0;
    return _d->_cachedDistanceMapping;
}

DistanceMappingEntry::DistanceMappingEntry(quint32 frameNumber, float metersPerFrame):
    _frameNumber(frameNumber), _metersPerFrame(metersPerFrame)
{
}

DistanceMappingEntry::DistanceMappingEntry():
    _frameNumber(0), _metersPerFrame(0.0f)
{
}


