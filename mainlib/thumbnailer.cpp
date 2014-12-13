#include "thumbnailer.h"

#include <QtCore/QDir>
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QtDebug>

extern "C" {
#include <gst/gst.h>
}

namespace
{
const QSize THUMBNAIL_SIZE(1280, 720);
/*!
 * \brief GSTREAMER_CAPS caps (capabilities) for gstreamer.
 *
 * These capabilities will convert frames to 24-bits RGB, 1280x720 (16:9). Because the pixel-aspect-ratio
 * is set to 1/1, the pixels will stay "square". If the original movie is not in 16:9 format, black bars
 * will be added on the sides or on top or bottom.
 */
const QString GSTREAMER_CAPS = QString("video/x-raw,format=RGB,width=%1,height=%2,pixel-aspect-ratio=1/1").arg(THUMBNAIL_SIZE.width()).arg(THUMBNAIL_SIZE.height());

/**
 * @brief template for the gstreamer pipeline.
 *
 * This will create a gstreamer pipeline that opens a video file, converts the video file to the CAPS mentioned
 * above and outputs to an appsink element. This element will be used to save a frame to an image.
 */
const QString GSTREAMER_PIPELINE_TEMPLATE = QString("uridecodebin uri=%2 ! videoconvert ! videoscale ! appsink name=sink caps=\"%1\"").arg(GSTREAMER_CAPS);

/**
 * @brief Create a thumbnail for the rlv.
 * @param rlv the rlv to create the thumbnail for.
 */
bool createThumbnailFor(const RealLifeVideo &rlv, const QString& filename);

/**
 * @brief create the pipeline for the video of a certain RealLifeVideo
 * @param rlv the RealLifeVideo to create the pipeline for.
 * @return the pipeline, or nullptr if unable to create the pipeline.
 */
GstElement *createPipeline(const RealLifeVideo& rlv);

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
QPixmap getStillImage(GstElement* pipeline);
/**
 * @brief convert a GstSample to a QPixmap.
 * @param sample the sample.
 * @return  a still image. This will return a null pixmap (pixmap.isNull() will be true) when not successful.
 */
QPixmap convertSampleToPixmap(GstSample *sample);
}

Thumbnailer::Thumbnailer(QObject *parent): QObject(parent)
{
    _cacheDirectory = thumbnailDirectory();
    createCacheDirectoryIfNotExists();
}

QPixmap Thumbnailer::thumbnailFor(const RealLifeVideo &rlv)
{
    if (doesThumbnailExistsFor(rlv)) {
        return loadThumbnailFor(rlv);
    }

    _thumbnailCreationFutureWatcher = new QFutureWatcher<bool>;
    connect(_thumbnailCreationFutureWatcher, SIGNAL(finished()), this, SLOT(pixmapCreated()));
    _thumbnailCreationFuture = QtConcurrent::run(createThumbnailFor, rlv, cacheFilePathFor(rlv));
    _thumbnailCreationFutureWatcher->setFuture(_thumbnailCreationFuture);

    QPixmap emptyPixmap(THUMBNAIL_SIZE);
    emptyPixmap.fill(Qt::black);
    return emptyPixmap;
}

void Thumbnailer::createCacheDirectoryIfNotExists()
{
    QDir cacheDir(_cacheDirectory);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

QString Thumbnailer::cacheFilePathFor(const RealLifeVideo &rlv)
{
    QString filename = QString("%1.jpg").arg(rlv.name());
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

void Thumbnailer::pixmapCreated()
{
    _thumbnailCreationFutureWatcher->deleteLater();
    bool ok = _thumbnailCreationFuture.resultAt(0);
    if (ok) {
        emit pixmapUpdated();
    }
}

bool Thumbnailer::doesThumbnailExistsFor(const RealLifeVideo &rlv)
{
    QFile cacheFile(cacheFilePathFor(rlv));
    return cacheFile.exists() && cacheFile.size() > 0;
}

QPixmap Thumbnailer::loadThumbnailFor(const RealLifeVideo &rlv)
{
    return QPixmap(cacheFilePathFor(rlv));
}

namespace {

bool createThumbnailFor(const RealLifeVideo &rlv, const QString& filename)
{
    qDebug() << QString("generating cache file for %1").arg(rlv.name());

    GstElement* pipeline = createPipeline(rlv);
    if (!pipeline) {
        qWarning("Unable to create pipeline. Aborting creation of thumbnail");
        return false;
    }

    if (!openVideoFile(pipeline)) {
        qWarning("Unable to open video file for %s. Aborting creation of video file", qPrintable(rlv.name()));
        return false;
    }

    QPixmap pixmap = getStillImage(pipeline);
    if (pixmap.isNull()) {
        qWarning("Unable to get sample for video %s, Aborting creation of video file", qPrintable(rlv.name()));
    } else {
        pixmap.save(filename);
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));

    return true;
}


GstElement* createPipeline(const RealLifeVideo& rlv)
{
    GError* error = nullptr;
    QString fileUri(QUrl::fromLocalFile(rlv.videoInformation().videoFilename()).toEncoded());
    QString pipelineDescription = QString(GSTREAMER_PIPELINE_TEMPLATE).arg(fileUri);

    GstElement* pipeline = gst_parse_launch(pipelineDescription.toStdString().c_str(), &error);

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

QPixmap getStillImage(GstElement* pipeline)
{
    GstElement* sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");
    GstSample *sample = nullptr;
    /* get the preroll buffer from appsink, this block untils appsink really
       * prerolls */
    g_signal_emit_by_name (sink, "pull-preroll", &sample, nullptr);

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

    QImage image(map.data, THUMBNAIL_SIZE.width(), THUMBNAIL_SIZE.height(), QImage::Format_RGB888);
    gst_buffer_unmap (buffer, &map);

    return QPixmap::fromImage(image);
}

}
