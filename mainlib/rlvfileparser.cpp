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

#include "rlvfileparser.h"

#include <QDir>
#include <QFileInfo>
#include <QtDebug>

namespace tacxfile
{
QString fromUtf16(const char* string, size_t size) {
    QByteArray bytes;

    size_t i = 0;
    while(string[i] != '\0' && i < size) {
        bytes.append(string[i]);
        bytes.append(string[i+1]);
        i += 2;
    }
    return QTextCodec::codecForName("UTF-16")->toUnicode(bytes);
}

class DistanceMappingEntry {
public:
    explicit DistanceMappingEntry(quint32 frameNumber, float metersPerFrame) {
        _frameNumber = frameNumber;
        _metersPerFrame = metersPerFrame;
    }

    quint32 frameNumber() const { return _frameNumber; }
    float metersPerFrame() const { return _metersPerFrame; }
private:
    quint32 _frameNumber;
    float _metersPerFrame;
};
}

RlvFileParser::RlvFileParser(const QList<QFileInfo> &pgmfFiles, const QList<QFileInfo>& videoFiles): TacxFileParser(),
    _pgmfFiles(pgmfFiles), _videoFiles(videoFiles)
{
}

QString RlvFileParser::findVideoFilename(const QList<QFileInfo>& videoFiles, const QString& rlvVideoFilename)
{
    QString normalizedVideoFilename;
    // if rlv video filename contains a path, get only the last part of it.
    normalizedVideoFilename = rlvVideoFilename.split("\\").last();

    foreach(const QFileInfo& fileInfo, videoFiles) {
        if (fileInfo.fileName().toLower() == normalizedVideoFilename.toLower())
            return fileInfo.canonicalFilePath();
    }

    return QString();
}

/**
 * Find the pgmf file for an rlv file. This checks if any of the pgmf files that were
 * found has the same name (minus the extension) as the rlv file. If found, returns
 * the path, else, returns an empty string.
 */
QFileInfo RlvFileParser::findPgmfFile(QFile &rlvFile)
{
    QFileInfo fileInfo(rlvFile);

    QString baseName = fileInfo.baseName();
    for (QFileInfo pgmfFileInfo: _pgmfFiles) {
        if (pgmfFileInfo.baseName() == baseName) {
            return pgmfFileInfo;
        }
    }
    return QFileInfo();
}

RealLifeVideo RlvFileParser::parseRlvFile(QFile &rlvFile)
{
    qDebug() << rlvFile.fileName();
    if (!rlvFile.open(QIODevice::ReadOnly))
        return RealLifeVideo();

    QFile pgmfFile(findPgmfFile(rlvFile).filePath());
    if (!pgmfFile.open(QIODevice::ReadOnly))
        return RealLifeVideo();

    Profile profile = PgmfFileParser().readProfile(pgmfFile);

    QList<Course> courses;
    QString name = QFileInfo(rlvFile).baseName();
    VideoInformation videoInformation(QString("Unknown"), 0.0);
    QList<DistanceMappingEntry> distanceMapping;

    tacxfile::header_t header = readHeaderBlock(rlvFile);
    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
        tacxfile::info_t infoBlock = readInfoBlock(rlvFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
            break;
        if (infoBlock.fingerprint == 2010) {
            tacxfile::generalRlv_t generalRlv = readGeneralRlvBlock(rlvFile);
            QString videoFilename = findVideoFilename(_videoFiles, generalRlv.filename());
            videoInformation = VideoInformation(videoFilename, generalRlv.frameRate);
        }
        else if (infoBlock.fingerprint == 2020) {
            distanceMapping = readFrameDistanceMapping(rlvFile, infoBlock.numberOfRecords);
        }
        else if (infoBlock.fingerprint == 2040) {
            courses = readCourseInformation(rlvFile, infoBlock.numberOfRecords);
        }
        else {
            rlvFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
        }
    }
    return RealLifeVideo(name, "Tacx", videoInformation, courses, distanceMapping, profile);
}

tacxfile::header_t TacxFileParser::readHeaderBlock(QFile &rlvFile)
{
    tacxfile::header_t headerBlock;
    rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

    return headerBlock;
}

tacxfile::info_t TacxFileParser::readInfoBlock(QFile &rlvFile)
{
    tacxfile::info_t infoBlock;
    rlvFile.read((char*) &infoBlock, sizeof(infoBlock));
    return infoBlock;
}

tacxfile::generalRlv_t RlvFileParser::readGeneralRlvBlock(QFile &rlvFile)
{
    tacxfile::generalRlv_t generalBlock;
    rlvFile.read((char*) &generalBlock._filename, 522);
    rlvFile.read((char*) &generalBlock.frameRate, sizeof(float));
    rlvFile.read((char*) &generalBlock.originalRunWeight, sizeof(float));
    rlvFile.read((char*) &generalBlock.frameOffset, sizeof(qint32));

    return generalBlock;
}

QList<Course> RlvFileParser::readCourseInformation(QFile &rlvFile, qint32 count)
{
    QList<Course> courses;
    for (qint32 i = 0; i < count; i++) {
        tacxfile::rlvCourseInformation_t courseInfoBlock;
        memset(&courseInfoBlock, 0, sizeof(courseInfoBlock));
        rlvFile.read((char*) &courseInfoBlock.start, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock.end, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock._courseName, 66);
        rlvFile.read((char*) &courseInfoBlock._textFilename, 522);

        courses.append(Course(courseInfoBlock.courseName(), courseInfoBlock.start, courseInfoBlock.end));
    }
    return courses;
}

QList<DistanceMappingEntry> RlvFileParser::readFrameDistanceMapping(QFile &rlvFile, qint32 count)
{
    QList<tacxfile::DistanceMappingEntry> mappings;
    mappings.reserve(count);

    for (qint32 i = 0; i < count; i++) {
        tacxfile::frameDistanceMapping_t frameDistanceMappingBlock;
        rlvFile.read((char*) &frameDistanceMappingBlock, sizeof(frameDistanceMappingBlock));
        mappings << tacxfile::DistanceMappingEntry(frameDistanceMappingBlock.frameNumber, frameDistanceMappingBlock.metersPerFrame);
    }
    return calculateRlvDistanceMappings(mappings);
}

QList<DistanceMappingEntry> RlvFileParser::calculateRlvDistanceMappings(const QList<tacxfile::DistanceMappingEntry> &tacxDistanceMappings) const
{
    float currentDistance = 0;
    float lastMetersPerFrame = 0;

    QList<DistanceMappingEntry> distanceMappings;
    if (!tacxDistanceMappings.isEmpty()) {
        quint32 lastFrameNumber = tacxDistanceMappings[0].frameNumber();
        QListIterator<tacxfile::DistanceMappingEntry> it(tacxDistanceMappings);
        while(it.hasNext()) {
            const tacxfile::DistanceMappingEntry& entry = it.next();
            quint32 nrFrames = entry.frameNumber() - lastFrameNumber;
            currentDistance += nrFrames * lastMetersPerFrame;
            distanceMappings.append(DistanceMappingEntry(currentDistance, entry.frameNumber(), entry.metersPerFrame()));

            lastMetersPerFrame = entry.metersPerFrame();
            lastFrameNumber = entry.frameNumber();
        }
    }
    return distanceMappings;
}


Profile PgmfFileParser::readProfile(QFile &pgmfFile)
{
    tacxfile::header_t header = readHeaderBlock(pgmfFile);
    tacxfile::generalPgmf_t generalBlock;
    QList<tacxfile::program_t> profileBlocks;

    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
        tacxfile::info_t infoBlock = readInfoBlock(pgmfFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
            break;
        else if (infoBlock.fingerprint == 1010)
            generalBlock = readGeneralPgmfInfo(pgmfFile);
        else if (infoBlock.fingerprint == 1020)
            profileBlocks = readProgram(pgmfFile, infoBlock.numberOfRecords);
        else
            pgmfFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
    }

    std::vector<ProfileEntry> profile;
    if (generalBlock.powerSlopeOrHr == 1) {
        float currentDistance = 0;
        float currentAltitude = 0;
        QListIterator<tacxfile::program_t> it(profileBlocks);
        while(it.hasNext()) {
            tacxfile::program_t item = it.next();
            profile.push_back(ProfileEntry(currentDistance, item.powerSlopeHeartRate, currentAltitude));
            currentDistance += item.durationDistance;
            currentAltitude += item.powerSlopeHeartRate * .01f * item.durationDistance;
        }
    }
    ProfileType type = (ProfileType) generalBlock.powerSlopeOrHr;

    return Profile(type, generalBlock.startAltitude, std::move(profile));
}

tacxfile::generalPgmf_t PgmfFileParser::readGeneralPgmfInfo(QFile &pgmfFile)
{
    tacxfile::generalPgmf_t generalBlock;
    pgmfFile.read((char*) &generalBlock.checksum, sizeof(qint32));
    pgmfFile.read((char*) &generalBlock._courseName, 34);
    pgmfFile.read((char*) &generalBlock.powerSlopeOrHr, sizeof(generalBlock) - offsetof(tacxfile::generalPgmf_t, powerSlopeOrHr));

    return generalBlock;
}

QList<tacxfile::program_t> PgmfFileParser::readProgram(QFile &pgmfFile, quint32 count)
{
    QList<tacxfile::program_t> programBlocks;
    programBlocks.reserve(count);

    for (quint32 i = 0; i < count; ++i) {
        tacxfile::program_t programBlock;
        pgmfFile.read((char*) &programBlock, sizeof(programBlock));
        programBlocks << programBlock;
    }
    return programBlocks;
}
