/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "thumbnailer.h"

#include <memory>

#include <QtCore/QDir>
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QtDebug>
#include <QtGui/QFont>
#include <QtGui/QPainter>

extern "C" {
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
}

namespace
{
/**
 * @brief Size for default empty images.
 */
const QSize DEFAULT_IMAGE_SIZE(1920, 1080);

/**
 * @brief template for the gstreamer pipeline.
 *
 * This will create a gstreamer pipeline that opens a video file, converts the frames to RGB
 * and outputs to an appsink element. This appsink element will be used to save a frame to an image.
 * The frames will have the same resolution as the original video.
 */
const QString GSTREAMER_PIPELINE_TEMPLATE = QString("uridecodebin uri=%2 ! videoconvert ! videoscale ! appsink name=sink caps=\"video/x-raw,format=RGB\"");

/**
 * @brief Create a thumbnail for the rlv.
 * @param rlv the rlv to create the thumbnail for.
 * @return the pixmap created
 */
QPixmap createThumbnailFor(RealLifeVideo &rlv, const qreal distance, const QString& filename, QPixmap defaultPixmap);

/**
 * @brief Custom deleter for smart pointers to GstElement* pipelines.
 */
struct GstPipelineDeleter {
    void operator()(GstElement* pipeline)
    {
        qDebug() << "freeing gstreamer pipeline";
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (pipeline));
    }
};
/**
 * @brief unique pointer for pipelines. Use this so you don't have to delete pipelines yourself.
 */
typedef std::unique_ptr<GstElement,GstPipelineDeleter> UniqueGstPipelinePtr;

/**
 * @brief create the pipeline for the video of a certain RealLifeVideo
 * @param rlv the RealLifeVideo to create the pipeline for.
 * @return the pipeline, or nullptr if unable to create the pipeline.
 */
UniqueGstPipelinePtr createPipeline(const RealLifeVideo& rlv);

/**
 * @brief open the pipeline and start the video.
 * @param pipeline the pipeline
 * @return true if opening the video succeeds, false otherwise.
 */
bool openVideoFile(GstElement* pipeline);

/**
 * @brief get a still from the pipeline.
 * @param pipeline the pipeline
 * @return a still image. This will return a null pixmap (pixmap.isNull() will be true) when not successful.
 */
QPixmap getStillImage(GstElement* pipeline, RealLifeVideo& rlv, const qreal distance);
/**
 * @brief convert a GstSample to a QPixmap.
 * @param sample the sample.
 * @return  a still image. This will return a null pixmap (pixmap.isNull() will be true) when not successful.
 */
QPixmap convertSampleToPixmap(GstSample *sample);

/**
 * @brief get the size (width & height) from a sample.
 * @param sample the sample
 * @return the size.
 */
QSize getSizeFromSample(GstSample* sample);
}

Thumbnailer::Thumbnailer(QObject *parent): QObject(parent)
{
    _cacheDirectory = thumbnailDirectory();
    createCacheDirectoryIfNotExists();

    _emptyPixmap = QPixmap(DEFAULT_IMAGE_SIZE);
    _emptyPixmap.fill(Qt::black);

    QFont font;
    font.setPointSize(72);
    QPainter p(&_emptyPixmap);

    p.setFont(font);
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    p.setPen(Qt::white);
    p.drawText(_emptyPixmap.rect(), QString(tr("Loading screenshot")), textOption);
}

QPixmap Thumbnailer::thumbnailFor(RealLifeVideo &rlv, const qreal distance)
{
    if (doesThumbnailExistsFor(rlv, distance)) {
        return loadThumbnailFor(rlv, distance);
    }

    QFutureWatcher<QPixmap>* watcher = new QFutureWatcher<QPixmap>(this);
    connect(watcher, &QFutureWatcher<QPixmap>::finished, watcher, [rlv, distance, watcher,this]() {
        qDebug() << "pixmap ready for" << rlv.name();
        emit pixmapUpdated(rlv, distance, watcher->future().result());
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run(createThumbnailFor, rlv, distance, cacheFilePathFor(rlv, distance), _emptyPixmap));

    return _emptyPixmap;
}

void Thumbnailer::createCacheDirectoryIfNotExists()
{
    QDir cacheDir(_cacheDirectory);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

QString Thumbnailer::cacheFilePathFor(const RealLifeVideo &rlv, const qreal distance)
{
    QString filename = QString("%1_%2.jpg").arg(rlv.name()).arg(distance);
    return _cacheDirectory.absoluteFilePath(filename);
}

QDir Thumbnailer::thumbnailDirectory()
{
    QStringList paths = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    if (paths.isEmpty()) {
        return QDir("/tmp/thumbnails");
    } else {
        return QDir(QString("%1/thumbnails").arg(paths[0]));
    }
}

bool Thumbnailer::doesThumbnailExistsFor(const RealLifeVideo &rlv, const qreal distance)
{
    QFile cacheFile(cacheFilePathFor(rlv, distance));
    return cacheFile.exists() && cacheFile.size() > 0;
}

QPixmap Thumbnailer::loadThumbnailFor(const RealLifeVideo &rlv, const qreal distance)
{
    QString path = cacheFilePathFor(rlv, distance);
    return QPixmap(path);
}

// the following functions are in anonymous namespace so they're not exported.
namespace {

QPixmap createThumbnailFor(RealLifeVideo &rlv, const qreal distance, const QString& filename, QPixmap defaultPixmap)
{
    qDebug() << QString("generating cache file for %1").arg(rlv.name());

    // wrap pipeline in a UniqueGstPipelinePtr to make sure it's cleaned always.
    UniqueGstPipelinePtr pipeline = createPipeline(rlv);
    if (!pipeline) {
        qWarning("Unable to create pipeline. Aborting creation of thumbnail");
        return defaultPixmap;
    }

    if (!openVideoFile(pipeline.get())) {
        qWarning("Unable to open video file for %s. Aborting creation of video file", qPrintable(rlv.name()));
        return defaultPixmap;
    }

    QPixmap pixmap = getStillImage(pipeline.get(), rlv, distance);
    if (pixmap.isNull()) {
        qWarning("Unable to get sample for video %s, Aborting creation of video file", qPrintable(rlv.name()));
        return defaultPixmap;
    } else {
        pixmap.save(filename);
    }
    return pixmap;
}


UniqueGstPipelinePtr createPipeline(const RealLifeVideo& rlv)
{
    GError* error = nullptr;
    QString fileUri(QUrl::fromLocalFile(rlv.videoInformation().videoFilename()).toEncoded());
    QString pipelineDescription = QString(GSTREAMER_PIPELINE_TEMPLATE).arg(fileUri);

    UniqueGstPipelinePtr pipeline(gst_parse_launch(pipelineDescription.toStdString().c_str(), &error));
    if (error != nullptr) {
        qWarning("Unable to create pipeline %s for video file %s", error->message, qPrintable(fileUri));
        return nullptr;
    }
    return pipeline;
}

bool openVideoFile(GstElement* pipeline)
{
    GstStateChangeReturn ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
    switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
        qWarning("failed to open video file");
        return false;
    default:
        break;
    }

    ret = gst_element_get_state (pipeline, nullptr, nullptr, 1 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qWarning("failed to start playing video file");
        return false;
    }
    return true;
}

QPixmap getStillImage(GstElement* pipeline, RealLifeVideo& rlv, const qreal distance)
{
    GstElement* sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");
    gst_element_set_state(sink, GST_STATE_PAUSED);

    gst_app_sink_pull_preroll(GST_APP_SINK(sink));
    gint64 nanoSeconds;
    gst_element_query_duration(pipeline, GST_FORMAT_TIME, &nanoSeconds);
    rlv.setDuration(nanoSeconds / 1000);
    // first frame might be empty (black), so take 5th frame as minimum.
    quint32 frameNr = qMax(5u, rlv.frameForDistance(distance));

    qreal time = frameNr / rlv.videoInformation().frameRate();
    gst_element_seek_simple(pipeline, GST_FORMAT_TIME, static_cast<GstSeekFlags>(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE), time * GST_SECOND);

    GstState state;
    gst_element_get_state(pipeline, &state, NULL, GST_CLOCK_TIME_NONE);
    GstSample *sample = gst_app_sink_pull_preroll(GST_APP_SINK(sink));


    QPixmap pixmap;
    if (sample) {
        pixmap = convertSampleToPixmap(sample);
        gst_sample_unref (sample);
    } else {
        qWarning("unable to get a sample");
    }
    return pixmap;
}

QPixmap convertSampleToPixmap(GstSample *sample)
{
    GstBuffer* buffer = gst_sample_get_buffer (sample);
    GstMapInfo map;
    gst_buffer_map (buffer, &map, GST_MAP_READ);
    QSize size = getSizeFromSample(sample);
    QImage image(map.data, size.width(), size.height(), QImage::Format_RGB888);
    gst_buffer_unmap (buffer, &map);

    return QPixmap::fromImage(image);
}

QSize getSizeFromSample(GstSample* sample)
{
    GstCaps* caps = gst_sample_get_caps(sample);
    GstStructure* structure = gst_caps_get_structure(caps, 0);
    gint width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    return QSize(width, height);
}

}