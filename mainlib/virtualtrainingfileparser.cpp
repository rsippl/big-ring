#include "virtualtrainingfileparser.h"

#include <QtCore/QtDebug>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamAttributes>

namespace {
QString readSingleAttribute(const QXmlStreamAttributes& attributes, const QString attributeName) {
    for (auto attribute: attributes) {
        if (attribute.name().toString() == attributeName) {
            return attribute.value().toString();
        }
    }
    return QString();
}
}
namespace indoorcycling {

namespace virtualtrainingfileparser {
struct ProfileEntry {
    float distance;
    float altitude;
};
struct DistanceMappingEntry {
    float frame;
    float distance;
};
}
VirtualTrainingFileParser::VirtualTrainingFileParser(const QList<QFileInfo> &videoFiles, QObject *parent) :
    QObject(parent), _videoFilesInfo(videoFiles)
{
}

RealLifeVideo VirtualTrainingFileParser::parseVirtualTrainingFile(QFile &inputFile) const
{
    inputFile.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&inputFile);

    return parseXml(reader);
}

RealLifeVideo VirtualTrainingFileParser::parseXml(QXmlStreamReader &reader) const
{
    QString name;
    QString videoFilePath;
    float frameRate = 0;
    Profile profile;
    QList<Course> courses;
    QList<DistanceMappingEntry> distanceMappings;

    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "title") {
            name = readSingleAttribute(reader.attributes(), "en");
        } else if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "video-file-path") {
            QString videoFileName = reader.readElementText();
            videoFilePath = findVideoFile(videoFileName);
        } else if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "framerate") {
            frameRate = reader.readElementText().toFloat();
        } else if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "altitudes") {
            QList<ProfileEntry> profileEntries = convertProfileEntries(readProfileEntries(reader));
            profile = Profile(ProfileType::SLOPE, 0, profileEntries);
        } else if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "segments") {
            courses = readCourses(reader);
        } else if (currentTokenType == QXmlStreamReader::StartElement && reader.name() == "mappings") {
            distanceMappings = convertDistanceMappings(readDistanceMappings(reader));
        } else {
//            qDebug() << "unknown token" << reader.name();
        }

    }
    if (reader.hasError()) {
        qDebug() << "error!" << reader.errorString();
        // Error handling..?
        return RealLifeVideo();
    }
    VideoInformation videoInformation(videoFilePath, frameRate);

    return RealLifeVideo(name, videoInformation, courses, distanceMappings, profile);
}

QList<virtualtrainingfileparser::ProfileEntry> VirtualTrainingFileParser::readProfileEntries(QXmlStreamReader &reader) const
{
    QList<virtualtrainingfileparser::ProfileEntry> profileEntries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement && reader.name() == "altitude") {
            virtualtrainingfileparser::ProfileEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.altitude = readSingleAttribute(reader.attributes(), "height").toFloat();
            profileEntries.append(entry);
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "altitudes") {
            break;
        }
    }
    return profileEntries;
}

QList<ProfileEntry> VirtualTrainingFileParser::convertProfileEntries(const QList<virtualtrainingfileparser::ProfileEntry> &virtualTrainingProfileEntries) const
{
    QList<ProfileEntry> profileEntries;

    virtualtrainingfileparser::ProfileEntry *lastEntry = nullptr;
    float currentDistance;
    float currentAltitude;
    for (virtualtrainingfileparser::ProfileEntry profileEntry: virtualTrainingProfileEntries) {
        if (lastEntry) {
            float segmentDistance = profileEntry.distance - currentDistance;
            float segmentAltitudeDifference = profileEntry.altitude - currentAltitude;

            float slope = segmentAltitudeDifference / segmentDistance;
            float slopeInPercent = slope * 100.0f;

            profileEntries.append(ProfileEntry(segmentDistance, currentDistance, slopeInPercent, currentAltitude));
        }
        currentDistance = profileEntry.distance;
        currentAltitude = profileEntry.altitude;
        lastEntry = &profileEntry;
    }

    return profileEntries;
}

QString VirtualTrainingFileParser::findVideoFile(QString filename) const
{
    for (const QFileInfo &fileInfo: _videoFilesInfo) {
        if (fileInfo.fileName().toLower() == filename.toLower()) {
            return fileInfo.absoluteFilePath();
        }
    }
    return "";
}

QList<Course> VirtualTrainingFileParser::readCourses(QXmlStreamReader &reader) const
{
    QList<Course> courses;
    while(!reader.atEnd()) {
        if (reader.readNext() == QXmlStreamReader::StartElement && reader.name() == "segment") {
            QString name = readSingleAttribute(reader.attributes(), "name");
            float start = readSingleAttribute(reader.attributes(), "start").toFloat();
            float end = readSingleAttribute(reader.attributes(), "end").toFloat();

            courses.append(Course(name, start, end));
        }
    }
    return courses;
}

QList<virtualtrainingfileparser::DistanceMappingEntry> VirtualTrainingFileParser::readDistanceMappings(QXmlStreamReader &reader) const
{
    QList<virtualtrainingfileparser::DistanceMappingEntry> entries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement && reader.name() == "mapping") {
            virtualtrainingfileparser::DistanceMappingEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.frame = readSingleAttribute(reader.attributes(), "frame").toFloat();
            entries.append({entry});
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "mappings") {
            break;
        }
    }
    return entries;

}

QList<DistanceMappingEntry> VirtualTrainingFileParser::convertDistanceMappings(const QList<virtualtrainingfileparser::DistanceMappingEntry> &vcEntries) const
{
    QList<DistanceMappingEntry> mappings;

    virtualtrainingfileparser::DistanceMappingEntry *lastEntry = nullptr;
    float currentDistance;
    quint32 currentFrame;
    for (virtualtrainingfileparser::DistanceMappingEntry distanceMappingEntry: vcEntries) {
        if (lastEntry) {
            float segmentDistance = distanceMappingEntry.distance - currentDistance;
            quint32 frameDifference = distanceMappingEntry.frame - currentFrame;

            float metersPerFrame = segmentDistance / frameDifference;

            mappings.append(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
        }
        currentDistance = distanceMappingEntry.distance;
        currentFrame = distanceMappingEntry.frame;
        lastEntry = &distanceMappingEntry;
    }
    return mappings;
}
}
