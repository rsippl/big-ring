#include "gpxfileparser.h"

#include "videoinforeader.h"
#include <cmath>

#include <QtCore/QDateTime>
#include <QtCore/QtDebug>
#include <QtCore/QTextStream>
#include <QtPositioning/QGeoCoordinate>

namespace {
const double DEGREES_TO_RADIANS_FACTOR = M_PI / 180.0;

QString readSingleAttribute(const QXmlStreamAttributes& attributes, const QString& attributeName) {
    for (auto attribute: attributes) {
        if (attribute.name().toString() == attributeName) {
            return attribute.value().toString();
        }
    }
    return QString();
}

bool isElement(const QXmlStreamReader::TokenType currentTokenType, const QXmlStreamReader& reader, const QString &elementName)
{
    return (currentTokenType == QXmlStreamReader::StartElement && reader.name() == elementName);
}
}

namespace indoorcycling {
namespace gpxfileparser {
struct TrackPoint {
    QGeoCoordinate position;
    double elevation;
    QDateTime dateTime;
    int frameNumber;
};
}

GpxFileParser::GpxFileParser(const QList<QFileInfo> &videoFiles, QObject *parent) :
    QObject(parent), _videoFiles(videoFiles)
{
}

RealLifeVideo GpxFileParser::parseGpxFile(QFile &inputFile) const
{
    const QFileInfo* videoFile = videoFileForGpsFile(inputFile);
    if (videoFile == nullptr) {
        return RealLifeVideo();
    }
    inputFile.open(QIODevice::ReadOnly);
    const QString xml = QTextStream(&inputFile).readAll();
    QXmlStreamReader reader(xml);
    return parseXml(inputFile, *videoFile, reader);
}

const QFileInfo* GpxFileParser::videoFileForGpsFile(const QFile &inputFile) const
{
    const QString baseName = QFileInfo(inputFile).baseName();
    for (const QFileInfo &videoFileInfo: _videoFiles) {
        const QString videoBaseName = videoFileInfo.baseName();
        if (baseName == videoBaseName) {
            return &videoFileInfo;
        }
    }
    return nullptr;
}

RealLifeVideo GpxFileParser::parseXml(const QFile &inputFile,
                                      const QFileInfo &videoFileInfo,
                                      QXmlStreamReader &reader) const
{
    QString name = QFileInfo(inputFile).baseName();
    float frameRate = VideoInfoReader().videoInfoForVideo(videoFileInfo).frameRate;

    QList<gpxfileparser::TrackPoint> trackPoints;
    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "trkpt")) {
            trackPoints.append(readTrackPoint(reader));
        }
    }

    QList<gpxfileparser::TrackPoint> trackPointsWithFrameNumbers = addFrameNumbers(trackPoints, frameRate);

    VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    Profile profile(ProfileType::SLOPE, 0.0f, convertProfileEntries(trackPoints));
    QList<Course> courses = { Course("Complete Distance", 0, profile.totalDistance()) };
    QList<DistanceMappingEntry> distanceMappings = convertDistanceMappings(trackPointsWithFrameNumbers);
    RealLifeVideo rlv(name, "GPX", videoInformation, courses, distanceMappings, profile);
    return rlv;
}

gpxfileparser::TrackPoint GpxFileParser::readTrackPoint(QXmlStreamReader &reader) const
{
    gpxfileparser::TrackPoint trackPoint;
    double latitude = readSingleAttribute(reader.attributes(), "lat").toDouble();
    double longitude = readSingleAttribute(reader.attributes(), "lon").toDouble();
    trackPoint.position = QGeoCoordinate(latitude, longitude);

    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "ele")) {
            trackPoint.elevation = reader.readElementText().toFloat();
        } else if (isElement(currentTokenType, reader, "time")) {
            trackPoint.dateTime = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
        } else if (currentTokenType == QXmlStreamReader::EndElement && reader.name() == "trkpt") {
            break;
        }
    }

    return trackPoint;
}

QList<ProfileEntry> GpxFileParser::convertProfileEntries(const QList<gpxfileparser::TrackPoint> &trackPoints) const
{
    QList<ProfileEntry> profileEntries;

    const gpxfileparser::TrackPoint *lastEntry = nullptr;
    float currentDistance;
    float currentElevation;
    for (const gpxfileparser::TrackPoint &trackPoint: trackPoints) {
        float segmentDistance = 0;
        if (lastEntry) {
            segmentDistance = static_cast<float>(lastEntry->position.distanceTo(trackPoint.position));
            float segmentAltitudeDifference = trackPoint.elevation - currentElevation;

            float slope = segmentAltitudeDifference / segmentDistance;
            float slopeInPercent = slope * 100.0f;

            profileEntries.append(ProfileEntry(segmentDistance, currentDistance, slopeInPercent, currentElevation));
        }
        currentDistance += segmentDistance;
        currentElevation = trackPoint.elevation;
        lastEntry = &trackPoint;
    }

    return profileEntries;
}

QList<DistanceMappingEntry> GpxFileParser::convertDistanceMappings(
        const QList<gpxfileparser::TrackPoint> &trackPoints) const
{
    QList<DistanceMappingEntry> mappings;

    const gpxfileparser::TrackPoint *lastTrackPoint = nullptr;
    float currentDistance = 0;
    quint32 currentFrame = 0;
    for (const gpxfileparser::TrackPoint &trackPoint: trackPoints) {
        float segmentDistance = 0;
        if (lastTrackPoint) {
            segmentDistance = static_cast<float>(lastTrackPoint->position.distanceTo(trackPoint.position));
            quint32 frameDifference = trackPoint.frameNumber - currentFrame;

            float metersPerFrame = segmentDistance / frameDifference;

            mappings.append(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
        }
        currentDistance += segmentDistance;
        currentFrame = trackPoint.frameNumber;
        lastTrackPoint = &trackPoint;
    }
    return mappings;
}

QList<gpxfileparser::TrackPoint> GpxFileParser::addFrameNumbers(const QList<gpxfileparser::TrackPoint> &trackPoints,
                                                                float frameRate) const
{
    QList<gpxfileparser::TrackPoint> trackPointsWithFrameNumbers;

    if (trackPoints.isEmpty()) {
        return trackPointsWithFrameNumbers;
    }

    QDateTime startTime = trackPoints[0].dateTime;
    for(const auto &trackPoint: trackPoints) {
        gpxfileparser::TrackPoint trackPointWithFrameNumber = trackPoint;
        qint64 differenceFromStart = startTime.msecsTo(trackPointWithFrameNumber.dateTime);
        trackPointWithFrameNumber.frameNumber = static_cast<quint32>((differenceFromStart * frameRate) / 1000);
        trackPointsWithFrameNumbers.append(trackPointWithFrameNumber);
    }
    return trackPointsWithFrameNumbers;
}
}
