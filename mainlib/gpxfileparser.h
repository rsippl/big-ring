#ifndef GPXFILEPARSER_H
#define GPXFILEPARSER_H

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

#include "reallifevideo.h"

namespace indoorcycling {
namespace gpxfileparser {
class TrackPoint;
}

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
    gpxfileparser::TrackPoint readTrackPoint(QXmlStreamReader &reader) const;
    QList<ProfileEntry> convertProfileEntries(const QList<gpxfileparser::TrackPoint> &trackPoints) const;
    QList<DistanceMappingEntry> convertDistanceMappings(const QList<gpxfileparser::TrackPoint> &trackPoints) const;

    QList<gpxfileparser::TrackPoint> addFrameNumbers(const QList<gpxfileparser::TrackPoint> &trackPoints,
                                                     float frameRate) const;
    double distanceBetweenPoints(const gpxfileparser::TrackPoint &p1, const gpxfileparser::TrackPoint &p2) const;
    const QList<QFileInfo> _videoFiles;
};
}

#endif // GPXFILEPARSER_H
