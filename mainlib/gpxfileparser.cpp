#include "gpxfileparser.h"

#include "videoinforeader.h"
#include <cmath>

#include <QtCore/QDateTime>
#include <QtCore/QtDebug>
#include <QtCore/QTextStream>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>

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

double smooth(double previous, double current, double next) {
    return 0.3 * previous + 0.4 * current + 0.3 * next;
}
}

namespace indoorcycling {
namespace gpxfileparser {
struct TrackPoint {
    QGeoPositionInfo positionInfo;
    int frameNumber;

    QGeoCoordinate coordinate() const { return positionInfo.coordinate(); }
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

    QList<gpxfileparser::TrackPoint> trackPointsWithFrameNumbers = addFrameNumbers(smoothTrack(trackPoints), frameRate);

    VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    Profile profile(ProfileType::SLOPE, 0.0f, convertProfileEntries(trackPointsWithFrameNumbers));
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
    double altitude = 0.0;
    QDateTime dateTime;

    while(!reader.atEnd()) {
        QXmlStreamReader::TokenType currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "ele")) {
            altitude = reader.readElementText().toDouble();
        } else if (isElement(currentTokenType, reader, "time")) {
            dateTime = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
        } else if (currentTokenType == QXmlStreamReader::EndElement && reader.name() == "trkpt") {
            break;
        }
    }

    trackPoint.positionInfo = QGeoPositionInfo(QGeoCoordinate(latitude, longitude, altitude), dateTime);
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
            segmentDistance = static_cast<float>(lastEntry->coordinate().distanceTo(trackPoint.coordinate()));
            float segmentAltitudeDifference = trackPoint.coordinate().altitude() - currentElevation;

            float slope = segmentAltitudeDifference / segmentDistance;
            float slopeInPercent = slope * 100.0f;

            profileEntries.append(ProfileEntry(segmentDistance, currentDistance, slopeInPercent, currentElevation));
        }
        currentDistance += segmentDistance;
        currentElevation = trackPoint.coordinate().altitude();
        lastEntry = &trackPoint;
    }

    return profileEntries;
}

/**
 * @brief smooth the track. We will only smooth altitude, not lat-lon, because that will mess with distances.
 * @param trackPoints the trackpoints.
 * @return the track with the altitudes smoothed.
 */
QList<gpxfileparser::TrackPoint> GpxFileParser::smoothTrack(const QList<gpxfileparser::TrackPoint> &trackPoints) const
{
    QList<gpxfileparser::TrackPoint> smoothedTrackPoints = trackPoints;
    for (int i = 0; i < 20; ++i) {
        smoothedTrackPoints = smoothTrackPoints(smoothedTrackPoints);
    }
    return smoothedTrackPoints;
}

QList<gpxfileparser::TrackPoint> GpxFileParser::smoothTrackPoints(const QList<gpxfileparser::TrackPoint> &trackPoints) const
{
    QList<gpxfileparser::TrackPoint> smoothedTrackPoints;
    if (!trackPoints.isEmpty()) {
        smoothedTrackPoints.append(trackPoints[0]);
        for(int i = 1; i < trackPoints.size() - 1; ++i) {
            smoothedTrackPoints.append(smoothSingleTrackPoint(trackPoints[i -1],
                                       trackPoints[i], trackPoints[i + 1]));
        }
        smoothedTrackPoints.append(trackPoints.last());
    }
    Q_ASSERT(smoothedTrackPoints.size() == trackPoints.size());
    return smoothedTrackPoints;
}

gpxfileparser::TrackPoint GpxFileParser::smoothSingleTrackPoint(const gpxfileparser::TrackPoint &previousPoint, const gpxfileparser::TrackPoint &point, const gpxfileparser::TrackPoint &nextPoint) const
{
    QGeoCoordinate coordinate = point.coordinate();

    coordinate.setAltitude(smooth(previousPoint.coordinate().altitude(),
                                               point.coordinate().altitude(),
                                               nextPoint.coordinate().altitude()));

    gpxfileparser::TrackPoint newTrackPoint = point;
    newTrackPoint.positionInfo = QGeoPositionInfo(coordinate, point.positionInfo.timestamp());
    return newTrackPoint;
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
            segmentDistance = static_cast<float>(lastTrackPoint->coordinate().distanceTo(trackPoint.coordinate()));
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

    QDateTime startTime = trackPoints[0].positionInfo.timestamp();
    for(const auto &trackPoint: trackPoints) {
        gpxfileparser::TrackPoint trackPointWithFrameNumber = trackPoint;
        qint64 differenceFromStart = startTime.msecsTo(trackPointWithFrameNumber.positionInfo.timestamp());
        trackPointWithFrameNumber.frameNumber = static_cast<quint32>((differenceFromStart * frameRate) / 1000);
        trackPointsWithFrameNumbers.append(trackPointWithFrameNumber);
    }
    return trackPointsWithFrameNumbers;
}
}
