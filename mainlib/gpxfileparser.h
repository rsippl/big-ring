#ifndef GPXFILEPARSER_H
#define GPXFILEPARSER_H

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

#include <QtPositioning/QGeoPositionInfo>

#include "reallifevideo.h"

namespace indoorcycling {

class GpxFileParser : public QObject
{
    Q_OBJECT
public:
    explicit GpxFileParser(const QList<QFileInfo>& videoFiles, QObject *parent = 0);
    /**
     * @brief parse a GPX rlv file.
     * @param inputFile the file to parse.
     * @return a RealLifeVideo. If parsing failed, the resulting RealLifeVideo will not be valid (check with isValid()).
     */
    RealLifeVideo parseGpxFile(QFile &inputFile) const;
private:
    const QFileInfo* videoFileForGpsFile(const QFile &inputFile) const;
    RealLifeVideo parseXml(const QFile &inputFile, const QFileInfo &videoFileInfo, QXmlStreamReader &reader) const;
    QGeoPositionInfo readTrackPoint(QXmlStreamReader &reader) const;
    QList<ProfileEntry> convertProfileEntries(const QList<QGeoPositionInfo> &trackPoints) const;
    QList<QGeoPositionInfo> smoothTrack(const QList<QGeoPositionInfo> &trackPoints) const;
    QList<QGeoPositionInfo> smoothTrackPoints(const QList<QGeoPositionInfo> &trackPoints) const;
    QGeoPositionInfo smoothSingleTrackPoint(
            const QGeoPositionInfo &previousPoint,
            const QGeoPositionInfo &point,
            const QGeoPositionInfo &nextPoint) const;

    QList<DistanceMappingEntry> convertDistanceMappings(float frameRate, const QList<QGeoPositionInfo> &trackPoints) const;

    const QList<QFileInfo> _videoFiles;
};
}

#endif // GPXFILEPARSER_H
