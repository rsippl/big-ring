#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QtCore/QDir>
#include <QtCore/QFuture>
#include <QtCore/QFutureWatcher>
#include <QtGui/QPixmap>
#include "reallifevideo.h"

/**
 * @brief Class that handles the making of thumbnails for videos by
 * opening the video files and taking the first frame. That frame
 * is converted to a pixmap object.
 */
class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(QObject* parent = 0);

    QPixmap thumbnailFor(const RealLifeVideo& rlv);
    QString cacheFilePathFor(const RealLifeVideo& rlv);
    static QDir thumbnailDirectory();

signals:
    void pixmapUpdated(QPixmap pixmap);

private slots:
    void pixmapCreated();

private:
    /*!
     * \brief Create the cache directory in which the thumbnails will be stored, if it does not exist yet.
     */
    void createCacheDirectoryIfNotExists();
    /**
     * @brief check if a thumnail exists for a RealLifeVideo.
     * @param rlv the video..
     * @return true if there is a thumbnail.
     */
    bool doesThumbnailExistsFor(const RealLifeVideo& rlv);

    /**
     * Load the thumbnail for a RealLifeVideo.
     */
    QPixmap loadThumbnailFor(const RealLifeVideo& rlv);

    /**
     * @brief The directory where thumbnails are cached.
     */
    QDir _cacheDirectory;

    QPixmap _emptyPixmap;
};

#endif // THUMBNAILER_H
