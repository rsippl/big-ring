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

#include "videoplayer.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QtDebug>
#include <QtCore/QThread>

#include "framebuffer.h"
#include "framecopyingvideoreader.h"
#include "openglpainter2.h"

VideoPlayer::VideoPlayer(QGLWidget *paintWidget, QObject *parent) :
    QObject(parent), _videoReader(new FrameCopyingVideoReader), _videoReaderThread(new QThread),
    _loadState(LoadState::NONE), _currentFrameNumber(0u), _lastFrameLoaded(0), _stepSize(1)
{
    _painter = new OpenGLPainter2(paintWidget, this);

    _videoReader->moveToThread(_videoReaderThread);
    connect(_videoReaderThread, &QThread::finished, _videoReaderThread, &QThread::deleteLater);
    connect(_videoReaderThread, &QThread::finished, _videoReader, &FrameCopyingVideoReader::deleteLater);
    _videoReaderThread->start();

    connect(_videoReader, &FrameCopyingVideoReader::videoOpened, this, &VideoPlayer::setVideoOpened);
    connect(_videoReader, &FrameCopyingVideoReader::seekReady, this, &VideoPlayer::setSeekReady);
    connect(_videoReader, &FrameCopyingVideoReader::frameCopied, this, &VideoPlayer::setFrameLoaded);
    connect(_painter, &OpenGLPainter2::frameNeeded, this, &VideoPlayer::setFrameNeeded);

}

VideoPlayer::~VideoPlayer()
{
    _videoReaderThread->quit();
}

bool VideoPlayer::isReadyToPlay()
{
    return _loadState == LoadState::DONE;
}

void VideoPlayer::stop()
{
    // noop
}

void VideoPlayer::stepToFrame(quint32 frameNumber)
{
    qDebug() << "stepping to frame" << frameNumber;
    if (_loadState == LoadState::DONE) {
        if (frameNumber > _lastFrameLoaded) {
            qWarning("Requesting to show a frame (%ud) that is not loaded yet. last = %lld!. Step size %d", frameNumber, _lastFrameLoaded, _stepSize);
            _stepSize = 10;
            _painter->fillBuffers();
            return;
        } else {
            _stepSize = frameNumber - _currentFrameNumber;
            if (_stepSize == 0) {
                return;
            }
            if (_stepSize > 5) {
                qDebug() << "step size is too big:" << _stepSize << "current frame number:" << _currentFrameNumber;
            }
        }
        _painter->showFrame(frameNumber);
        updateCurrentFrameNumber(frameNumber);
        emit updateVideo();
    } else {
        qDebug() << "stepping when video not ready. Ignoring.";
    }
}


void VideoPlayer::loadVideo(QString uri)
{
    _videoReader->openVideoFile(uri);
    _stepSize = 1;
    updateLoadState(LoadState::VIDEO_LOADING);
}

bool VideoPlayer::seekToFrame(quint32 frameNumber)
{
    if (_loadState == LoadState::VIDEO_LOADED || _loadState == LoadState::DONE) {
        _videoReader->seekToFrame(frameNumber);
        updateLoadState(LoadState::SEEKING);
        return true;
    }
    return false;
}

void VideoPlayer::displayCurrentFrame(QPainter *painter, QRectF rect, Qt::AspectRatioMode aspectRatioMode)
{
    _painter->paint(painter, rect, aspectRatioMode);
}

void VideoPlayer::setVideoOpened(const QString &, const QSize& videoSize, const QSize &videoFrameSize, const qint64 numberOfFrames)
{
    _painter->setVideoSize(videoSize, videoFrameSize);
    updateLoadState(LoadState::VIDEO_LOADED);
    emit videoLoaded(numberOfFrames);
}

void VideoPlayer::setSeekReady(qint64 frameNumber)
{
    _currentFrameNumber = frameNumber;
    updateLoadState(LoadState::DONE);
    _painter->fillBuffers();
    QTimer::singleShot(2000, this, [this] {
        emit seekDone();
    });
}

void VideoPlayer::setFrameLoaded(int index, qint64 frameNumber, const QSize &frameSize)
{
    qDebug() << "frame" << frameNumber << "loaded";
    _lastFrameLoaded = frameNumber;
    _painter->setFrameLoaded(index, frameNumber, frameSize);
}

void VideoPlayer::setFrameNeeded(const std::weak_ptr<FrameBuffer> &frameBuffer)
{
    if (auto locked = frameBuffer.lock()) {
        if (locked->mappedBufferPointer()) {
            _videoReader->copyNextFrame(frameBuffer, _stepSize - 1);
        }
    }
}

void VideoPlayer::updateCurrentFrameNumber(const quint32 frameNumber)
{
    _currentFrameNumber = frameNumber;
}

void VideoPlayer::updateLoadState(const VideoPlayer::LoadState loadState)
{
    _loadState = loadState;
}


