#include "virtualcyclingfileparser.h"

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

namespace virtualcyclingfileparser {
struct ProfileEntry {
    float distance;
    float altitude;
};
struct DistanceMappingEntry {
    float frame;
    float distance;
};
}
VirtualCyclingFileParser::VirtualCyclingFileParser(const QList<QFileInfo> &videoFiles, QObject *parent) :
    QObject(parent), _videoFilesInfo(videoFiles)
{
}

std::unique_ptr<RealLifeVideo> VirtualCyclingFileParser::parseVirtualCyclingFile(QFile &inputFile) const
{
    inputFile.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&inputFile);

    return parseXml(reader);
}

std::unique_ptr<RealLifeVideo> VirtualCyclingFileParser::parseXml(QXmlStreamReader &reader) const
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
        return std::unique_ptr<RealLifeVideo>();
    }
    VideoInformation videoInformation(videoFilePath, frameRate);

    return std::unique_ptr<RealLifeVideo>(new RealLifeVideo(name, videoInformation, courses,
                                                            distanceMappings, profile));
}

QList<virtualcyclingfileparser::ProfileEntry> VirtualCyclingFileParser::readProfileEntries(QXmlStreamReader &reader) const
{
    QList<virtualcyclingfileparser::ProfileEntry> profileEntries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement && reader.name() == "altitude") {
            virtualcyclingfileparser::ProfileEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.altitude = readSingleAttribute(reader.attributes(), "height").toFloat();
            profileEntries.append(entry);
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "altitudes") {
            break;
        }
    }
    return profileEntries;
}

QList<ProfileEntry> VirtualCyclingFileParser::convertProfileEntries(const QList<virtualcyclingfileparser::ProfileEntry> virtualCyclingProfileEntries) const
{
    QList<ProfileEntry> profileEntries;

    virtualcyclingfileparser::ProfileEntry *lastVcProfileEntry = nullptr;
    float currentDistance;
    float currentAltitude;
    for (virtualcyclingfileparser::ProfileEntry vcProfileEntry: virtualCyclingProfileEntries) {
        if (lastVcProfileEntry) {
            float segmentDistance = vcProfileEntry.distance - currentDistance;
            float segmentAltitudeDifference = vcProfileEntry.altitude - currentAltitude;

            float slope = segmentAltitudeDifference / segmentDistance;
            float slopeInPercent = slope * 100.0f;

            profileEntries.append(ProfileEntry(segmentDistance, currentDistance, slopeInPercent, currentAltitude));
        }
        currentDistance = vcProfileEntry.distance;
        currentAltitude = vcProfileEntry.altitude;
        lastVcProfileEntry = &vcProfileEntry;
    }
    return profileEntries;
}

QString VirtualCyclingFileParser::findVideoFile(QString filename) const
{
    for (const QFileInfo &fileInfo: _videoFilesInfo) {
        if (fileInfo.fileName().toLower() == filename.toLower()) {
            return fileInfo.absoluteFilePath();
        }
    }
    return "";
}

QList<Course> VirtualCyclingFileParser::readCourses(QXmlStreamReader &reader) const
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

QList<virtualcyclingfileparser::DistanceMappingEntry> VirtualCyclingFileParser::readDistanceMappings(QXmlStreamReader &reader) const
{
    QList<virtualcyclingfileparser::DistanceMappingEntry> entries;
    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType tokenType = reader.readNext();
        if (tokenType == QXmlStreamReader::StartElement && reader.name() == "mapping") {
            virtualcyclingfileparser::DistanceMappingEntry entry;
            entry.distance = readSingleAttribute(reader.attributes(), "distance").toFloat();
            entry.frame = readSingleAttribute(reader.attributes(), "frame").toFloat();
            entries.append({entry});
        } else if (tokenType == QXmlStreamReader::EndElement && reader.name() == "mappings") {
            break;
        }
    }
    return entries;

}

QList<DistanceMappingEntry> VirtualCyclingFileParser::convertDistanceMappings(const QList<virtualcyclingfileparser::DistanceMappingEntry> &vcEntries) const
{
    QList<DistanceMappingEntry> mappings;

    virtualcyclingfileparser::DistanceMappingEntry *lastVcEntry = nullptr;
    float currentDistance;
    quint32 currentFrame;
    for (virtualcyclingfileparser::DistanceMappingEntry vcEntry: vcEntries) {
        if (lastVcEntry) {
            float segmentDistance = vcEntry.distance - currentDistance;
            quint32 frameDifference = vcEntry.frame - currentFrame;

            float metersPerFrame = segmentDistance / frameDifference;

            mappings.append(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
        }
        currentDistance = vcEntry.distance;
        currentFrame = vcEntry.frame;
        lastVcEntry = &vcEntry;
    }
    return mappings;
}
}
