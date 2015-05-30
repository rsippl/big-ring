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

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QtDebug>
#include <QtCore/QThread>
#include <QtGui/QFont>
#include <QtGui/QPainter>

#include "videoreader.h"

namespace
{
/**
 * @brief Size for default empty images.
 */
const QSize DEFAULT_IMAGE_SIZE(1920, 1080);

}

QPixmap Thumbnailer::createEmptyPixmap() const
{
    QPixmap emptyPixmap(DEFAULT_IMAGE_SIZE);
    emptyPixmap.fill(Qt::black);

    QFont font;
    font.setPointSize(72);
    QPainter p(&emptyPixmap);

    p.setFont(font);
    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    p.setPen(Qt::white);
    p.drawText(emptyPixmap.rect(), QString(tr("Loading screenshot")), textOption);

    return emptyPixmap;
}

Thumbnailer::Thumbnailer(QObject *parent): QObject(parent), _videoReader(new VideoReader)
{
    connect(_videoReader, &VideoReader::newFrameReady, this, &Thumbnailer::setNewFrame);
    _cacheDirectory = thumbnailDirectory();
    createCacheDirectoryIfNotExists();

    _emptyPixmap = createEmptyPixmap();

    QThread* videoReaderThread = new QThread;
    _videoReader->moveToThread(videoReaderThread);
    connect(this, &Thumbnailer::destroyed, _videoReader, &QThread::deleteLater);
    connect(videoReaderThread, &QThread::destroyed, _videoReader, &VideoReader::deleteLater);
    videoReaderThread->start();
}

Thumbnailer::~Thumbnailer()
{
    // empty
}

/**
 * @brief get the thumbnail for an rlv at a certain distance. If no thumbnail is ready yet, an empty thumbnail
 * will be returned. A generated thumbnail will later be emitted using a pixmapUpdated() signal.
 * @param rlv the rlv
 * @param distance the distance
 * @return a thumbnail, or an empty pixmap if no thumbnail is currently ready.
 */
QPixmap Thumbnailer::thumbnailFor(RealLifeVideo &rlv, const qreal distance)
{
    if (doesThumbnailExistsFor(rlv, distance)) {
        return loadThumbnailFor(rlv, distance);
    }

    _videoReader->createImageForFrame(rlv, distance);
    return _emptyPixmap;
}

void Thumbnailer::setNewFrame(const RealLifeVideo& rlv, const qreal distance, const QImage &frame)
{
    frame.save(cacheFilePathFor(rlv, distance));
    emit pixmapUpdated(rlv, distance, QPixmap::fromImage(frame));
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

