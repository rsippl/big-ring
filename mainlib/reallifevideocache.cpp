/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
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

#include "reallifevideocache.h"
#include "distancemappingentry.h"
#include "videoinformation.h"

#include <QtCore/QDataStream>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>

RealLifeVideoCache::RealLifeVideoCache(QObject *parent) :
    QObject(parent)
{
    //empty
}

std::unique_ptr<RealLifeVideo> RealLifeVideoCache::load(const QFile &rlvFile)
{
    QFileInfo rlvFileInfo(rlvFile);
    QFile cacheFile(absoluteFilenameForRlv(rlvFileInfo.fileName()));
    if (!cacheFile.exists()) {
        return std::unique_ptr<RealLifeVideo>();
    }
    QFileInfo cacheFileInfo(cacheFile);
    if (rlvFileInfo.lastModified() > cacheFileInfo.lastModified()) {
        return std::unique_ptr<RealLifeVideo>();
    }


    cacheFile.open(QIODevice::ReadOnly);
    QDataStream in(&cacheFile);

    quint32 magic;
    in >> magic;
    quint32 version;
    in >> version;

    QString rlvName;
    in >> rlvName;
    quint32 fileTypeAsInt;
    in >> fileTypeAsInt;
    const RealLifeVideoFileType fileType = static_cast<RealLifeVideoFileType>(fileTypeAsInt);

    qreal frameRate;
    in >> frameRate;
    QString videoFilename;
    in >> videoFilename;
    VideoInformation videoInformation(videoFilename, frameRate);

    std::vector<Course> courses = readCourses(in);
    std::vector<DistanceMappingEntry> distanceMappings = readDistanceMappings(in);
    Profile profile = readProfile(in);
    std::vector<InformationBox> informationBoxes = readInformationBoxes(in);

    return std::unique_ptr<RealLifeVideo>(new RealLifeVideo(rlvName, fileType, videoInformation, std::move(courses),
                                                            std::move(distanceMappings), profile, std::move(informationBoxes)));
}

void RealLifeVideoCache::saveRlv(const QFile &rlvFile, const RealLifeVideo &rlv)
{
    const QString filename = QFileInfo(rlvFile).fileName();
    QFile file(absoluteFilenameForRlv(filename));
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    out << (quint32) 0xAABBCCDD;
    out << (quint32) 1;

    out.setVersion(QDataStream::Qt_5_4);

    out << rlv.name();
    out << static_cast<quint32>(rlv.fileType());
    out << rlv.videoFrameRate();
    out << rlv.videoFilename();

    saveCourses(out, rlv.courses());
    saveDistanceMappings(out, rlv.distanceMappings());
    saveProfile(out, rlv.profile());
    saveInformationBoxes(out, rlv.informationBoxes());
}

QString RealLifeVideoCache::absoluteFilenameForRlv(const QString &name) const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
    QDir rlvCacheDir = QDir(QString("%1/rlv").arg(path));
    if (!rlvCacheDir.exists()) {
        rlvCacheDir.mkpath(".");
    }
    return rlvCacheDir.filePath(QString("%1.rlvdat").arg(name));
}

void RealLifeVideoCache::saveCourses(QDataStream &out, const std::vector<Course> &courses) const
{
    out << static_cast<quint32>(courses.size());
    for (const Course &course: courses) {
        out << course.name();
        out << static_cast<quint32>(course.type());
        out << course.start();
        out << course.end();
    }
}

std::vector<Course> RealLifeVideoCache::readCourses(QDataStream &in) const
{
    std::vector<Course> courses;
    quint32 numberOfCourses;
    in >> numberOfCourses;
    courses.reserve(numberOfCourses);
    for (auto i = 0u; i < numberOfCourses; ++i) {
        QString name;
        in >> name;
        quint32 typeAsInt;
        in >> typeAsInt;
        float start;
        in >> start;
        float end;
        in >> end;
        courses.push_back(Course(name, static_cast<Course::Type>(typeAsInt), start, end));
    }
    return courses;
}

void RealLifeVideoCache::saveDistanceMappings(QDataStream &out, const std::vector<DistanceMappingEntry> &distanceMappings) const
{
    out << static_cast<quint32>(distanceMappings.size());
    for (const DistanceMappingEntry &entry: distanceMappings) {
        out << entry.distance();
        out << entry.frameNumber();
        out << entry.metersPerFrame();
    }
}

std::vector<DistanceMappingEntry> RealLifeVideoCache::readDistanceMappings(QDataStream &in) const
{
    std::vector<DistanceMappingEntry> distanceMappings;
    quint32 numberOfDistanceMappings;
    in >> numberOfDistanceMappings;
    distanceMappings.reserve(numberOfDistanceMappings);
    for (auto i = 0u; i < numberOfDistanceMappings; ++i) {
        float distance;
        in >> distance;
        quint32 frameNumber;
        in >> frameNumber;
        float metersPerFrame;
        in >> metersPerFrame;
        distanceMappings.push_back(DistanceMappingEntry(distance, frameNumber, metersPerFrame));
    }
    return distanceMappings;
}

void RealLifeVideoCache::saveProfile(QDataStream &out, const Profile &profile) const
{
    out << static_cast<quint32>(profile.type());
    out << profile.startAltitude();
    saveProfileEntries(out, profile.entries());
}

Profile RealLifeVideoCache::readProfile(QDataStream &in)
{
    quint32 profileTypeAsInt;
    in >> profileTypeAsInt;
    float startAltitude;
    in >> startAltitude;
    std::vector<ProfileEntry> entries = readProfileEntries(in);
    return Profile(static_cast<ProfileType>(profileTypeAsInt), startAltitude, std::move(entries));
}

void RealLifeVideoCache::saveProfileEntries(QDataStream &out, const std::vector<ProfileEntry> &entries) const
{
    out << static_cast<quint32>(entries.size());
    for (const ProfileEntry &entry: entries) {
        out << entry.distance();
        out << entry.altitude();
        out << entry.slope();
    }
}

std::vector<ProfileEntry> RealLifeVideoCache::readProfileEntries(QDataStream &in) const
{
    quint32 numberOfEntries;
    in >> numberOfEntries;

    std::vector<ProfileEntry> entries;
    entries.reserve(numberOfEntries);
    for (auto i = 0u; i < numberOfEntries; ++i) {
        float distance, altitude, slope;
        in >> distance >> altitude >> slope;
        entries.push_back(ProfileEntry(distance, slope, altitude));
    }
    return entries;
}

void RealLifeVideoCache::saveInformationBoxes(QDataStream &out, const std::vector<InformationBox> &entries) const
{
    out << static_cast<quint32>(entries.size());
    for (const InformationBox &entry: entries) {
        out << entry.distance();
        out << entry.frameNumber();
        out << entry.imageFileInfo().absoluteFilePath();
        out << entry.message();
    }
}

std::vector<InformationBox> RealLifeVideoCache::readInformationBoxes(QDataStream &in) const
{
    quint32 numberOfEntries;
    in >> numberOfEntries;

    std::vector<InformationBox> entries;
    entries.reserve(numberOfEntries);
    for (auto i = 0u; i < numberOfEntries; ++i) {
        float distance;
        quint32 frameNumber;
        QString imageFilePath;
        QString message;

        in >> distance;
        in >> frameNumber;
        in >> imageFilePath;
        in >> message;

        entries.push_back(InformationBox(frameNumber, distance, message, QFileInfo(imageFilePath)));
    }
    return entries;
}
