#ifndef THUMBNAILER_H
#define THUMBNAILER_H

#include <QtCore/QDir>
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

    QString cacheFilePathFor(const RealLifeVideo& rlv);
    static QDir thumbnailDirectory();

signals:

public slots:
    /**
     * @brief Create a thumbnail for the rlv.
     * @param rlv the rlv to create the thumbnail for.
     */
    void createThumbnailFor(const RealLifeVideo &rlv);

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
     * @brief The directory where thumbnails are cached.
     */
    QDir _cacheDirectory;
};

#endif // THUMBNAILER_H
