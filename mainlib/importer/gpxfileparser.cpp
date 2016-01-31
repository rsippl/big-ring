#include "gpxfileparser.h"

#include "model/distancemappingentry.h"
#include "model/videoinformation.h"
#include "video/videoinforeader.h"

#include <deque>

#include <QtCore/QDateTime>
#include <QtCore/QtDebug>
#include <QtCore/QTextStream>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>

namespace {

const int NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE = 5;
const float MINIMUM_SLOPE = -15.0;
const float MAXIMUM_SLOPE = 20.0;

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

    std::vector<GeoPosition> geoPositions = convertTrackPoints(trackPoints);

    VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    Profile profile(ProfileType::SLOPE, 0.0f, smoothProfile(convertProfileEntries(geoPositions)));
    std::vector<Course> courses = { Course("Complete Distance", 0, profile.totalDistance()) };
    std::vector<DistanceMappingEntry> distanceMappings = convertDistanceMappings(frameRate, trackPoints);
    RealLifeVideo rlv(name, RealLifeVideoFileType::GPX, videoInformation, std::move(courses),
                      std::move(distanceMappings), profile, std::move(std::vector<InformationBox>()),
                      std::move(geoPositions));
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

std::vector<GeoPosition> GpxFileParser::convertTrackPoints(const std::vector<QGeoPositionInfo> &positions) const
{
    qreal currentDistance = 0;
    std::vector<GeoPosition> geoPositions;
    const QGeoPositionInfo *lastEntry = nullptr;
    geoPositions.reserve(positions.size());
    for (const QGeoPositionInfo& position: positions) {
        if (lastEntry == nullptr) {
            geoPositions.push_back(GeoPosition(currentDistance, position.coordinate()));
        } else {
            const qreal segmentDistance = distanceBetweenPoints(*lastEntry, position);
            currentDistance += segmentDistance;
            geoPositions.push_back(GeoPosition(currentDistance, position.coordinate()));
        }
        lastEntry = &position;
    }
    return geoPositions;
}

std::vector<ProfileEntry> GpxFileParser::convertProfileEntries(const std::vector<GeoPosition> &trackPoints) const
{
    std::vector<ProfileEntry> profileEntries;

    const GeoPosition *lastEntry = nullptr;
    qreal currentDistance = 0;
    qreal currentElevation = 0;
    for (const GeoPosition &trackPoint: trackPoints) {
        qreal segmentDistance = 0;
        if (lastEntry) {
            segmentDistance = trackPoint.distance() - lastEntry->distance();
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
 * Smooth the profile by taking a running average for the slope of each entry.
 */
std::vector<ProfileEntry> GpxFileParser::smoothProfile(const std::vector<ProfileEntry> &profile) const
{
    std::vector<ProfileEntry> smoothedProfile;
    smoothedProfile.reserve(profile.size());

    for (auto it = profile.cbegin(); it != profile.end(); ++it) {
        // determine bounds of the interval that we use for averaging. Make sure not to go beyond the
        // begin or end of the vector.
        const auto averageEntriesBegin = std::max(profile.cbegin(), it - NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE / 2);
        const auto averageEntriesEnd = std::min(profile.cend(), it + NUMBER_OF_ITEMS_FOR_MOVING_AVERAGE / 2 + 1);

        // calculate the (simple) average
        const float averageSlope = std::accumulate(averageEntriesBegin, averageEntriesEnd, 0.0, [](const float sum, const ProfileEntry& entry) {
            return sum + qBound(MINIMUM_SLOPE, entry.slope(), MAXIMUM_SLOPE);
        }) / (averageEntriesEnd - averageEntriesBegin);

        const ProfileEntry& lastEntry = (smoothedProfile.empty()) ? *it : smoothedProfile.back();

        // determine altitude from distance and new slope. For the first entry, distanceDifference will be zero.
        const float distanceDifference = it->distance() - lastEntry.distance();
        const float newAltitude = lastEntry.altitude() + lastEntry.slope() * 0.01 * distanceDifference;

        smoothedProfile.push_back(ProfileEntry(it->distance(), averageSlope, newAltitude));
    }
    return smoothedProfile;
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
