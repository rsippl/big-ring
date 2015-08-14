#include "gpxfileparser.h"

#include "videoinforeader.h"

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

    QList<QGeoPositionInfo> trackPoints;
    QXmlStreamReader::TokenType currentTokenType;
    while (!reader.atEnd()) {
        currentTokenType = reader.readNext();
        if (isElement(currentTokenType, reader, "trkpt")) {
            trackPoints.append(readTrackPoint(reader));
        }
    }

    VideoInformation videoInformation(videoFileInfo.filePath(), frameRate);
    Profile profile(ProfileType::SLOPE, 0.0f, convertProfileEntries(smoothTrack(trackPoints)));
    QList<Course> courses = { Course("Complete Distance", 0, profile.totalDistance()) };
    QList<DistanceMappingEntry> distanceMappings = convertDistanceMappings(frameRate, trackPoints);
    RealLifeVideo rlv(name, "GPX", videoInformation, courses, distanceMappings, profile);
    return rlv;
}

QGeoPositionInfo GpxFileParser::readTrackPoint(QXmlStreamReader &reader) const
{
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

    return QGeoPositionInfo(QGeoCoordinate(latitude, longitude, altitude), dateTime);
}

QList<ProfileEntry> GpxFileParser::convertProfileEntries(const QList<QGeoPositionInfo> &trackPoints) const
{
    QList<ProfileEntry> profileEntries;

    const QGeoPositionInfo *lastEntry = nullptr;
    float currentDistance = 0;
    float currentElevation = 0;
    for (const QGeoPositionInfo &trackPoint: trackPoints) {
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
QList<QGeoPositionInfo> GpxFileParser::smoothTrack(const QList<QGeoPositionInfo> &trackPoints) const
{
    QList<QGeoPositionInfo> smoothedTrackPoints = trackPoints;
    for (int i = 0; i < 20; ++i) {
        smoothedTrackPoints = smoothTrackPoints(smoothedTrackPoints);
    }
    return smoothedTrackPoints;
}

QList<QGeoPositionInfo> GpxFileParser::smoothTrackPoints(const QList<QGeoPositionInfo> &trackPoints) const
{
    QList<QGeoPositionInfo> smoothedTrackPoints;
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

QGeoPositionInfo GpxFileParser::smoothSingleTrackPoint(const QGeoPositionInfo &previousPoint, const QGeoPositionInfo &point, const QGeoPositionInfo &nextPoint) const
{
    QGeoCoordinate coordinate = point.coordinate();

    coordinate.setAltitude(smooth(previousPoint.coordinate().altitude(),
                                               point.coordinate().altitude(),
                                               nextPoint.coordinate().altitude()));

    return QGeoPositionInfo(coordinate, point.timestamp());
}



QList<DistanceMappingEntry> GpxFileParser::convertDistanceMappings(
        float frameRate,
        const QList<QGeoPositionInfo> &trackPoints) const
{
    QList<DistanceMappingEntry> mappings;

    const QDateTime startTime = trackPoints[0].timestamp();

    const QGeoPositionInfo *lastTrackPoint = nullptr;
    float currentDistance = 0;
    quint32 currentFrame = 0;
    for (const QGeoPositionInfo &trackPoint: trackPoints) {
        float segmentDistance = 0;
        qint64 differenceFromStart = startTime.msecsTo(trackPoint.timestamp());
        quint32 frameNumberForPoint = static_cast<quint32>((differenceFromStart * frameRate) / 1000);
        if (lastTrackPoint) {
            segmentDistance = static_cast<float>(lastTrackPoint->coordinate().distanceTo(trackPoint.coordinate()));
            quint32 frameDifference = frameNumberForPoint - currentFrame;

            float metersPerFrame = segmentDistance / frameDifference;

            mappings.append(DistanceMappingEntry(currentDistance, currentFrame, metersPerFrame));
        }
        currentDistance += segmentDistance;
        currentFrame = frameNumberForPoint;
        lastTrackPoint = &trackPoint;
    }
    return mappings;
}
}
