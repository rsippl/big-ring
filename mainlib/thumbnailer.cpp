#include "thumbnailer.h"

#include <QtCore/QDir>

#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QtDebug>
extern "C" {
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
}

namespace
{
/*!
 * \brief GSTREAMER_CAPS caps (capabilities) for gstreamer.
 *
 * These capabilities will convert frames to 24-bits RGB, 1280x720 (16:9). Because the pixel-aspect-ratio
 * is set to 1/1, the pixels will stay "square". If the original movie is not in 16:9 format, black bars
 * will be added on the sides or on top or bottom.
 */
const QString GSTREAMER_CAPS = "video/x-raw,format=RGB,width=1280,height=720,pixel-aspect-ratio=1/1";

/**
 * @brief template for the gstreamer pipeline.
 *
 * This will create a gstreamer pipeline that opens a video file, converts the video file to the CAPS mentioned
 * above and outputs to an appsink element. This element will be used to save a frame to an image.
 */
const QString GSTREAMER_PIPELINE_TEMPLATE = QString("uridecodebin uri=%2 ! videoconvert ! videoscale ! appsink name=sink caps=\"%1\"").arg(GSTREAMER_CAPS);
}

Thumbnailer::Thumbnailer(QObject *parent): QObject(parent)
{
    _cacheDirectory = thumbnailDirectory();
    createCacheDirectoryIfNotExists();
}

void Thumbnailer::createCacheDirectoryIfNotExists()
{
    QDir cacheDir(_cacheDirectory);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
}

void Thumbnailer::createThumbnailFor(const RealLifeVideo &rlv)
{
    if (doesThumbnailExistsFor(rlv)) {
        qDebug() << QString("cache file exists for %1, no need to generate.").arg(rlv.name());
        return;
    }
    qDebug() << QString("cache file does not exist for %1, generating...").arg(rlv.name());

    GError* error = nullptr;
    QString pipelineDescription = QString(GSTREAMER_PIPELINE_TEMPLATE).arg(QUrl::fromLocalFile(rlv.videoInformation().videoFilename()).toString());

    GstElement* pipeline = gst_parse_launch(pipelineDescription.toStdString().c_str(), &error);

    if (error != nullptr) {
        qWarning("Unable to create pipeline %s", error->message);
        return;
    }
    GstElement* sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

    GstStateChangeReturn ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
    switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
        qWarning("failed to open video file");
        return;
    default:
        break;
    }

    ret = gst_element_get_state (pipeline, NULL, NULL, 1 * GST_SECOND);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("failed to play the file\n");
        exit (-1);
    }

    GstSample *sample;
    /* get the preroll buffer from appsink, this block untils appsink really
       * prerolls */
    g_signal_emit_by_name (sink, "pull-preroll", &sample, nullptr);

    if (sample) {
        /* get the snapshot buffer format now. We set the caps on the appsink so
         * that it can only be an rgb buffer. The only thing we have not specified
         * on the caps is the height, which is dependant on the pixel-aspect-ratio
         * of the source material */
        GstCaps *caps = gst_sample_get_caps (sample);
        if (!caps) {
            g_print ("could not get snapshot format\n");
            exit (-1);
        }
        GstStructure* s = gst_caps_get_structure (caps, 0);
        gint width, height;
        /* we need to get the final caps on the buffer to get the size */
        gboolean res = gst_structure_get_int (s, "width", &width);
        res |= gst_structure_get_int (s, "height", &height);
        if (!res) {
            g_print ("could not get snapshot dimension\n");
            exit (-1);
        } else {
            qDebug() << "size is " << QSize(width, height);
        }
        /* create pixmap from buffer and save, gstreamer video buffers have a stride
             * that is rounded up to the nearest multiple of 4 */
        GstBuffer* buffer = gst_sample_get_buffer (sample);
        GstMapInfo map;
        gst_buffer_map (buffer, &map, GST_MAP_READ);

        QImage image(map.data, width, height, QImage::Format_RGB888);
        if (!image.save(cacheFilePathFor(rlv))) {
            qWarning ("unable to save image for rlv %s", qPrintable(rlv.name()));
        }
        g_print("we should be saving here!\n");
        gst_buffer_unmap (buffer, &map);
        gst_sample_unref (sample);
    } else {
        qWarning("unable to get a sample");
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref (GST_OBJECT (pipeline));
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

bool Thumbnailer::doesThumbnailExistsFor(const RealLifeVideo &rlv)
{
    QFile cacheFile(cacheFilePathFor(rlv));
    return cacheFile.exists() && cacheFile.size() > 0;
}


