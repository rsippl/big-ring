#include "rlvfileparser.h"

#include <QFileInfo>
#include <QtDebug>

RlvFileParser::RlvFileParser(const QStringList& videoFilenames):
    _videoFilenames(videoFilenames)
{
}

QString RlvFileParser::findVideoFilename(const QStringList& videoFilenames, const QString& rlvVideoFilename)
{
    foreach(QString videoFilename, videoFilenames) {
        QFileInfo fileInfo(videoFilename);
        if (fileInfo.fileName().toLower() == rlvVideoFilename.toLower())
            return videoFilename;
    }
    return QString();
}

RealLiveVideo RlvFileParser::parseRlvFile(QFile &rlvFile)
{
    if (!rlvFile.open(QIODevice::ReadOnly))
        return RealLiveVideo();
    QList<Course> courses;
    QString name = QFileInfo(rlvFile).baseName();
    VideoInformation videoInformation(QString("Unknown"), 0.0);

    tacxfile::header_t header = readHeaderBlock(rlvFile);
    for(qint32 blockNr = 0; blockNr < header.numberOfBlocks; ++blockNr) {
        tacxfile::info_t infoBlock = readInfoBlock(rlvFile);
        if (infoBlock.fingerprint < 0 || infoBlock.fingerprint > 10000)
        break;
        qDebug() << "inforblock fingerprint: " << infoBlock.fingerprint;
        if (infoBlock.fingerprint == 2010) {
            tacxfile::generalRlv_t generalRlv = readGeneralRlvBlock(rlvFile);
            QString videoFilename = findVideoFilename(_videoFilenames, generalRlv.filename());
//            qDebug() << generalRlv.filename() << " => " << videoFilename;
            videoInformation = VideoInformation(videoFilename, generalRlv.frameRate);
        }
        else if (infoBlock.fingerprint == 2020) {
            readFrameDistanceMapping(rlvFile, infoBlock.numberOfRecords);
        }
        else if (infoBlock.fingerprint == 2040) {
            courses = readCourseInformation(rlvFile, infoBlock.numberOfRecords);
        }
        else {
            rlvFile.read(infoBlock.numberOfRecords * infoBlock.recordSize);
        }
    }
    return RealLiveVideo(name, videoInformation, courses);
}

tacxfile::header_t RlvFileParser::readHeaderBlock(QFile &rlvFile)
{
    tacxfile::header_t headerBlock;
    rlvFile.read((char*) &headerBlock, sizeof(headerBlock));

    return headerBlock;
}

tacxfile::info_t RlvFileParser::readInfoBlock(QFile &rlvFile)
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
//    qDebug() << generalBlock.toString();

    return generalBlock;
}

QList<Course> RlvFileParser::readCourseInformation(QFile &rlvFile, qint32 count)
{
    QList<Course> courses;
    for (qint32 i = 0; i < count; i++) {
        tacxfile::rlvCourseInformation_t courseInfoBlock;
        rlvFile.read((char*) &courseInfoBlock.start, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock.end, sizeof(float));
        rlvFile.read((char*) &courseInfoBlock._courseName, 66);
        rlvFile.read((char*) &courseInfoBlock._textFilename, 522);

        courses.append(Course(courseInfoBlock.courseName(), courseInfoBlock.start, courseInfoBlock.end));
    }
    return courses;
}

QList<tacxfile::frameDistanceMapping_t> RlvFileParser::readFrameDistanceMapping(QFile &rlvFile, qint32 count)
{
    QList<tacxfile::frameDistanceMapping_t> mappings;
    mappings.reserve(count);

    for (qint32 i = 0; i < count; i++) {
            tacxfile::frameDistanceMapping_t frameDistanceMappingBlock;
            rlvFile.read((char*) &frameDistanceMappingBlock, sizeof(frameDistanceMappingBlock));
            mappings << frameDistanceMappingBlock;
    }
    return mappings;
}
