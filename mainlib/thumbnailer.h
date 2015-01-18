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
