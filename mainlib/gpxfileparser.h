#ifndef GPXFILEPARSER_H
#define GPXFILEPARSER_H

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtCore/QXmlStreamReader>

#include <QtPositioning/QGeoPositionInfo>

#include "reallifevideo.h"

namespace indoorcycling {

/**
 * RealLifeVideo parser for GPX files.
 *
 * piece of a gpx file (Hahtennjoch.gpx) that is handled:
 *
 <gpx xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.1" xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd" creator="RoadMovieCreator" xmlns="http://www.topografix.com/GPX/1/1" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
 <trk>
  <trkseg>
   <trkpt lat="47.3253240064" lon="10.5277110357">
    <ele>972.7999877930</ele>
    <time>2014-09-29T17:24:46.000+00:00</time>
    <speed>8.93102</speed>
    <extensions>
     <mediatime>00:00:00.000</mediatime>
    </extensions>
   </trkpt>
   <trkpt lat="47.3253741302" lon="10.5278036557">
    <ele>972.7999877930</ele>
    <time>2014-09-29T17:24:47.000+00:00</time>
    <speed>8.93102</speed>
    <extensions>
     <mediatime>00:00:01.000</mediatime>
    </extensions>
   </trkpt>
 *
 * of this information, we use only the trkpt element. We take latitude (lat attribute),
 * longitude (lon attribute), altitude/elevation (<ele> element) and time (<time> element).
 *
 * It is assumed that the time of the first track point is at the same time as the start of
 * the video. We could also use the mediatime extension, if present.
 *
 * The <speed> element is not used currently. We calculate the number of meters per frame
 * by calculating the distance between two consecutive points and dividing that by the
 * frame rate of the video. To do this we have to open the video file and read the framerate
 * from it.
 *
 * Before reading information from the video file, the parser checks if there is a video
 * file present that has the same base name as the gpx file. For instance, for a gpx file
 * with the name NO_Tau.gpx, a video is searched with the base name NO_Tau, which would
 * match NO_Tau.avi or NO_Tau.mp4.
 */
class GpxFileParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create a GpxFileParser, with the list of video files that can be used.
     * @param videoFiles the video files that can be used as videos for RealLifeVideos.
     * @param parent optional QObject-parent.
     */
    explicit GpxFileParser(const QList<QFileInfo>& videoFiles, QObject *parent = 0);
    /**
     * @brief parse a GPX rlv file.
     *
     * Will search for a video file with the same base name and open the construct a RealLifeVideo from
     * this video file and the information in the gpx file.
     *
     * @param inputFile the file to parse.
     * @return a RealLifeVideo. If parsing failed, the resulting RealLifeVideo will not be valid (check with isValid()).
     */
    RealLifeVideo parseGpxFile(QFile &inputFile) const;

private:
    const QFileInfo *videoFileForGpsFile(const QFile &inputFile) const;
    RealLifeVideo parseXml(const QFile &inputFile, const QFileInfo &videoFileInfo, QXmlStreamReader &reader) const;
    QGeoPositionInfo readTrackPoint(QXmlStreamReader &reader) const;
    std::vector<ProfileEntry> convertProfileEntries(const std::vector<QGeoPositionInfo> &trackPoints) const;
    std::vector<QGeoPositionInfo> smoothTrack(const std::vector<QGeoPositionInfo> &trackPoints) const;
    std::vector<QGeoPositionInfo> smoothTrackPoints(const std::vector<QGeoPositionInfo> &trackPoints) const;
    QGeoPositionInfo smoothSingleTrackPoint(
            const QGeoPositionInfo &previousPoint,
            const QGeoPositionInfo &point,
            const QGeoPositionInfo &nextPoint) const;

    std::vector<DistanceMappingEntry> convertDistanceMappings(float frameRate,
                                                              const std::vector<QGeoPositionInfo> &trackPoints) const;

    const QList<QFileInfo> _videoFiles;
};
}

#endif // GPXFILEPARSER_H
