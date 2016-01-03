#include "gpxfileparser.h"

#include "model/distancemappingentry.h"
#include "model/videoinformation.h"
#include "video/videoinforeader.h"

#include <QtCore/QDateTime>
#include <QtCore/QtDebug>
#include <QtCore/QTextStream>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>

namespace {

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

double smooth(double previous, double current, double next) {
    return 0.3 * previous + 0.4 * current + 0.3 * next;
}

/**
 * @brief calculate the frame number for a trackpoint.
 * @param trackPoint the track point
 * @param startTime start time of the track
 * @param frameRate the frame rate.
 * @return a frame number.
 */
quint32 frameNumberForTrackPoint(const QGeoPositionInfo &trackPoint, const QDateTime &startTime, const float frameRate)
{
    const qint64 differenceFromStart = startTime.msecsTo(trackPoint.timestamp());
    return static_cast<quint32>((differenceFromStart * frameRate) / 1000);
}

}

namespace indoorcycling {

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

    std::vector<QGeoPositionInfo> trackPoints;
    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "trkpt")) {
            trackPoints.push_back(readTrackPoint(reader));
        }
    }

    VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    Profile profile(ProfileType::SLOPE, 0.0f, convertProfileEntries(smoothTrack(trackPoints)));
    std::vector<Course> courses = { Course("Complete Distance", 0, profile.totalDistance()) };
    std::vector<DistanceMappingEntry> distanceMappings = convertDistanceMappings(frameRate, trackPoints);
    RealLifeVideo rlv(name, RealLifeVideoFileType::GPX, videoInformation, std::move(courses),
                      std::move(distanceMappings), profile);
    return rlv;
}

QGeoPositionInfo GpxFileParser::readTrackPoint(QXmlStreamReader &reader) const
{
    double latitude = readSingleAttribute(reader.attributes(), "lat").toDouble();
    double longitude = readSingleAttribute(reader.attributes(), "lon").toDouble();
    double altitude = 0.0;
    double speed = 0.0;
    QDateTime dateTime;

    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "ele")) {
            altitude = reader.readElementText().toDouble();
        } else if (isElement(currentTokenType, reader, "time")) {
            dateTime = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
        } else if (isElement(currentTokenType, reader, "speed")) {
            speed = reader.readElementText().toDouble();
        } else if (currentTokenType == QXmlStreamReader::EndElement && reader.name() == "trkpt") {
            break;
        }
    }

    QGeoPositionInfo geoPositionInfo(QGeoCoordinate(latitude, longitude, altitude), dateTime);
    if (speed > 0.0) {
        geoPositionInfo.setAttribute(QGeoPositionInfo::GroundSpeed, speed);
    }
    return geoPositionInfo;
}

std::vector<ProfileEntry> GpxFileParser::convertProfileEntries(const std::vector<QGeoPositionInfo> &trackPoints) const
{
    std::vector<ProfileEntry> profileEntries;

    const QGeoPositionInfo *lastEntry = nullptr;
    qreal currentDistance = 0;
    qreal currentElevation = 0;
    for (const QGeoPositionInfo &trackPoint: trackPoints) {
        qreal segmentDistance = 0;
        if (lastEntry) {
            segmentDistance = distanceBetweenPoints(*lastEntry, trackPoint);
            qreal segmentAltitudeDifference = trackPoint.coordinate().altitude() - currentElevation;

            if (segmentDistance > 0) {
                float slope = segmentAltitudeDifference / segmentDistance;
                float slopeInPercent = slope * 100.0f;

                profileEntries.push_back(ProfileEntry(currentDistance, slopeInPercent, currentElevation));
            }
        }
        currentDistance += segmentDistance;
        currentElevation = trackPoint.coordinate().altitude();
        lastEntry = &trackPoint;
    }

    // if there was an entry, and there's at least one entry in profile entries, add a last entry
    // to profile entries with the same slope as the previous one
    if (lastEntry && !profileEntries.empty()) {
        profileEntries.push_back(ProfileEntry(currentDistance, profileEntries.back().slope(), lastEntry->coordinate().altitude()));
    }

    return profileEntries;
}

/**
 * @brief smooth the track. We will only smooth altitude, not lat-lon, because that will mess with distances.
 * @param trackPoints the trackpoints.
 * @return the track with the altitudes smoothed.
 */
std::vector<QGeoPositionInfo> GpxFileParser::smoothTrack(const std::vector<QGeoPositionInfo> &trackPoints) const
{
    std::vector<QGeoPositionInfo> smoothedTrackPoints = trackPoints;
    for (int i = 0; i < 20; ++i) {
        smoothedTrackPoints = smoothTrackPoints(smoothedTrackPoints);
    }
    return smoothedTrackPoints;
}

std::vector<QGeoPositionInfo> GpxFileParser::smoothTrackPoints(const std::vector<QGeoPositionInfo> &trackPoints) const
{
    std::vector<QGeoPositionInfo> smoothedTrackPoints;
    if (!trackPoints.empty()) {
        smoothedTrackPoints.push_back(trackPoints[0]);
        for(unsigned i = 1; i < trackPoints.size() - 1; ++i) {
            smoothedTrackPoints.push_back(smoothSingleTrackPoint(trackPoints[i -1],
                                       trackPoints[i], trackPoints[i + 1]));
        }
        smoothedTrackPoints.push_back(trackPoints.back());
    }
    Q_ASSERT(smoothedTrackPoints.size() == trackPoints.size());
    return smoothedTrackPoints;
}

QGeoPositionInfo GpxFileParser::smoothSingleTrackPoint(const QGeoPositionInfo &previousPoint, const QGeoPositionInfo &point, const QGeoPositionInfo &nextPoint) const
{
    QGeoCoordinate coordinate = point.coordinate();

    coordinate.setAltitude(smooth(previousPoint.coordinate().altitude(),
                                               point.coordinate().altitude(),
                                               nextPoint.coordinate().altitude()));

    return QGeoPositionInfo(coordinate, point.timestamp());
}

qreal GpxFileParser::distanceBetweenPoints(const QGeoPositionInfo &start, const QGeoPositionInfo &end) const
{
    if (start.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        const int durationMsecs = start.timestamp().msecsTo(end.timestamp());
        const qreal speedMps = start.attribute(QGeoPositionInfo::GroundSpeed);
        return durationMsecs * 0.001 * speedMps;
    } else {
        return start.coordinate().distanceTo(end.coordinate());
    }
}



std::vector<DistanceMappingEntry> GpxFileParser::convertDistanceMappings(
        float frameRate,
        const std::vector<QGeoPositionInfo> &trackPoints) const
{
    Q_ASSERT_X(!trackPoints.empty(), "convertDistanceMappings", "trackpoints should not be empty");

    std::vector<DistanceMappingEntry> mappings;

    const QDateTime startTime = trackPoints[0].timestamp();

    const QGeoPositionInfo *lastTrackPoint = nullptr;
    float currentDistance = 0;
    quint32 currentFrame = 0;
    for (const QGeoPositionInfo &trackPoint: trackPoints) {
        qreal segmentDistance = 0;
        quint32 frameNumberForPoint = frameNumberForTrackPoint(trackPoint, startTime, frameRate);
        if (lastTrackPoint) {
            segmentDistance = distanceBetweenPoints(*lastTrackPoint, trackPoint);
            quint32 frameDifference = frameNumberForPoint - currentFrame;
            if (frameDifference > 0) {
                float metersPerFrame = segmentDistance / frameDifference;
                mappings.push_back(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
            }
        }
        currentDistance += segmentDistance;
        currentFrame = frameNumberForPoint;
        lastTrackPoint = &trackPoint;
    }

    if (lastTrackPoint && !mappings.empty()) {
        quint32 frameNumber = frameNumberForTrackPoint(*lastTrackPoint, startTime, frameRate);
        mappings.push_back(DistanceMappingEntry(currentDistance, frameNumber, mappings.back().metersPerFrame()));
    }

    return mappings;
}

quint32 frameNumberForTrackPoint(const QGeoPositionInfo &trackPoint, const QDateTime &startTime, const float frameRate)
{
    const qint64 differenceFromStart = startTime.msecsTo(trackPoint.timestamp());
    return static_cast<quint32>((differenceFromStart * frameRate) / 1000);
}
}
