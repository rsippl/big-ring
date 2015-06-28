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

#ifndef REALLIVEVIDEO_H
#define REALLIVEVIDEO_H

#include <QList>
#include <QMap>
#include <QPair>
#include <QString>

#include <QtCore/QSharedPointer>

#include "profile.h"

class DistanceMappingEntry
{
public:
    explicit DistanceMappingEntry(quint32 frameNumber, float metersPerFrame);
    explicit DistanceMappingEntry();

    quint32 frameNumber() const { return _frameNumber; }
    float metersPerFrame() const { return _metersPerFrame; }

private:
    quint32 _frameNumber;
    float _metersPerFrame;
};

class Course
{
public:
    enum Type { Invalid, Normal, Unfinished, Custom };
    explicit Course(const QString& name, const Type type, float start, float end);
    explicit Course(const QString& name, float start, float end);
    explicit Course(float start, float end);
    explicit Course();

    bool isValid() const { return !(_type == Invalid);}
    const QString& name() const { return _name; }
    Type type() const { return _type; }
    /*! start distance of course in  meters */
    float start() const { return _start; }
    /*! end distance of course in meters */
    float end() const { return _end; }

private:
    QString _name;
    Type _type;
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


class RealLifeVideoData;

class RealLifeVideo
{
public:
    explicit RealLifeVideo(const QString& name, const VideoInformation& videoInformation, QList<Course>& courses,
                           QList<DistanceMappingEntry> distanceMappings, Profile profile);
    RealLifeVideo(const RealLifeVideo& other);
    explicit RealLifeVideo();

    bool isValid() const;
    ProfileType type() const;
    Profile& profile() const;
    const QString name() const;
    const VideoInformation& videoInformation() const;
    const QList<Course>& courses() const;

    /** Set the distance for the current unfinished run */
    void setUnfinishedRun(float distance);
    /** Add a new custom start point */
    void addStartPoint(float distance, const QString& name);

    void printDistanceMapping();
    /** Get the number or frames per meter for a certain distance */
    float metersPerFrame(const float distance);
    /** Get the exact frame for a distance. */
    quint32 frameForDistance(const float distance);
    /** Get the slope for a distance */
    float slopeForDistance(const float distance);
    //! Get the altitude for a distance */
    float altitudeForDistance(const float distance);
    /** Total distance */
    float totalDistance() const;
    /** Set duration of video, in number of frames */
    void setNumberOfFrames(quint64 numberOfFrames);

    bool operator==(const RealLifeVideo& other) const;
    static bool compareByName(const RealLifeVideo& rlv1, const RealLifeVideo& rlv2);
private:
    void calculateVideoCorrectionFactor(quint64 totalNrOfFrames);

    const QPair<float, DistanceMappingEntry> &findDistanceMappingEntryFor(const float distance);

    QSharedPointer<RealLifeVideoData> _d;

    float _lastKeyDistance;
    float _nextLastKeyDistance;
};
typedef QList<RealLifeVideo> RealLifeVideoList;

Q_DECLARE_METATYPE(RealLifeVideo)
#endif // REALLIVEVIDEO_H
