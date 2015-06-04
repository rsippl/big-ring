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

#ifndef OPENGLPAINTER_H
#define OPENGLPAINTER_H

#include <QObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLDebugMessage>
#include <QtGui/QOpenGLShaderProgram>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QOpenGLFunctions>

#include <array>
#include "FrameBuffer.h"

class OpenGLPainter2 : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLPainter2(QGLWidget* widget, QObject *parent = 0);
    virtual ~OpenGLPainter2();

    void paint(QPainter* painter, const QRectF& rect, Qt::AspectRatioMode aspectRatioMode);
    FrameBuffer getNextFrameBuffer();
    bool isAFrameNeeded() const;
signals:
    void buffersFull();
    void frameNeeded(const FrameBuffer& frameBuffer);
public slots:
    void setVideoSize(const QSize& frameSize);
    void setFrameLoaded(int index, const QSize& frameSize);
    void requestNewFrames();
    void showNextFrame();
    void reset();
private slots:
    void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);
private:
    void initializeOpenGL();
    void initYuv420PTextureInfo();
    void loadTextures();
    void loadPlaneTexturesFromPbo(int glTextureUnit, int textureUnit,
                                  int lineSize, int height, size_t offset);
    void adjustPaintAreas(const QRectF& targetRect, Qt::AspectRatioMode aspectRationMode);
    void initializeVertexCoordinatesBuffer(const QRectF &videoRect);
    void initializeTextureCoordinatesBuffer();
    quint32 combinedSizeOfTextures();


    QGLWidget* _widget;
    QOpenGLFunctions _glFunctions;
    bool _openGLInitialized;
    bool _firstFrameLoaded;
    bool _texturesInitialized;
    QSize _sourceSize;
    QRectF _targetRect;
    QRectF _blackBar1, _blackBar2;
    bool _sourceSizeDirty;
    Qt::AspectRatioMode _aspectRatioMode;

    GLuint _yTextureId;
    GLuint _uTextureId;
    GLuint _vTextureId;

    int _textureWidths[3];
    int _textureHeights[3];
    int _textureOffsets[3];

    QOpenGLBuffer _textureCoordinatesBuffer;
    QOpenGLBuffer _vertexBuffer;
    std::array<QOpenGLBuffer, 5> _pixelBuffers;
    quint32 _currentPixelBufferWritePosition;
    quint32 _currentPixelBufferReadPosition;
    quint32 _currentPixelBufferMappedPosition;

    QOpenGLShaderProgram _program;
};

#endif // OPENGLPAINTER_H
