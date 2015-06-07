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

#include "openglpainter2.h"

#include <QtGui/QOpenGLContext>
#include <QtCore/QtMath>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtGui/QOpenGLDebugLogger>
#include <QtGui/QOpenGLFunctions>

OpenGLPainter2::OpenGLPainter2(QGLWidget* widget, QObject *parent) :
    QObject(parent), _widget(widget), _openGLInitialized(false), _firstFrameLoaded(false),
    _texturesInitialized(false), _aspectRatioMode(Qt::KeepAspectRatioByExpanding),
    _currentPixelBufferWritePosition(0),
    _currentPixelBufferReadPosition(0),
    _currentPixelBufferMappedPosition(0)
{
    Q_INIT_RESOURCE(shaders);
}

OpenGLPainter2::~OpenGLPainter2()
{
    // empty
}

void OpenGLPainter2::loadTextures()
{
    loadPlaneTexturesFromPbo(GL_TEXTURE0, _yTextureId, _textureWidths[0], _textureHeights[0], (size_t) 0);
    loadPlaneTexturesFromPbo(GL_TEXTURE1, _uTextureId, _textureWidths[1], _textureHeights[1], _textureOffsets[1]);
    loadPlaneTexturesFromPbo(GL_TEXTURE2, _vTextureId, _textureWidths[2], _textureHeights[2], _textureOffsets[2]);

    _texturesInitialized = true;
}

void OpenGLPainter2::loadPlaneTexturesFromPbo(int glTextureUnit, int textureUnit,
                                           int lineSize, int height, size_t offset)
{
    glActiveTexture(glTextureUnit);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureUnit);
    qDebug() << "reading from pbo" << _currentPixelBufferReadPosition;
//    GLenum status = _glFunctions.glCheckFramebufferStatus(_pixelBuffers[_currentPixelBufferReadPosition].bufferId());
//    qDebug() << "status" << status << (status == GL_FRAMEBUFFER_COMPLETE);
    _pixelBuffers[_currentPixelBufferReadPosition].bind();

    if (_texturesInitialized) {
        glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, lineSize, height,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*) offset);
    } else {
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_LUMINANCE, lineSize, height,
                     0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (void*) offset);
    }

    _pixelBuffers[_currentPixelBufferReadPosition].release();
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void OpenGLPainter2::paint(QPainter *painter, const QRectF &rect, Qt::AspectRatioMode aspectRatioMode)
{
    QTime time;
    time.start();
    if (!_openGLInitialized) {
        initializeOpenGL();
    }
    if (!_firstFrameLoaded) {
        painter->fillRect(rect, Qt::black);
        return;
    }

    adjustPaintAreas(rect, aspectRatioMode);

    // if these are enabled, we need to reenable them after beginNativePainting()
    // has been called, as they may get disabled
    bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);
    _pixelBuffers[_currentPixelBufferReadPosition].bufferId();

    painter->beginNativePainting();

    if (stencilTestEnabled)
        glEnable(GL_STENCIL_TEST);
    if (scissorTestEnabled)
        glEnable(GL_SCISSOR_TEST);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    _program.bind();

    loadTextures();

    // set the texture and vertex coordinates using VBOs.
    _textureCoordinatesBuffer.bind();
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    _textureCoordinatesBuffer.release();

    _vertexBuffer.bind();
    glVertexPointer(2, GL_FLOAT, 0, 0);
    _vertexBuffer.release();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, _yTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_RECTANGLE, _uTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_RECTANGLE, _vTextureId);
    glActiveTexture(GL_TEXTURE0);

    _program.setUniformValue("yTex", 0);
    _program.setUniformValue("uTex", 1);
    _program.setUniformValue("vTex", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    _program.release();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    painter->endNativePainting();
    painter->fillRect(_blackBar1, Qt::black);
    painter->fillRect(_blackBar2, Qt::black);

    qDebug() << "Painting took" << time.elapsed() << "ms";
}

FrameBuffer OpenGLPainter2::getNextFrameBuffer()
{
    if (!_openGLInitialized) {
        _widget->context()->makeCurrent();
        initializeOpenGL();
    }
    _widget->context()->makeCurrent();

    qDebug() << QThread::currentThreadId() << "mapping pixel buffer" << _currentPixelBufferMappedPosition;
    QOpenGLBuffer currentMappedBuffer = _pixelBuffers[_currentPixelBufferMappedPosition];
    currentMappedBuffer.bind();
    FrameBuffer frameBuffer;
    frameBuffer.ptr = currentMappedBuffer.map(QOpenGLBuffer::WriteOnly);
    if (frameBuffer.ptr == nullptr) {
        qDebug() << "unable to map buffer";
    }
    frameBuffer.index = _currentPixelBufferMappedPosition;
    frameBuffer.frameSize = _sourceSize;
    _currentPixelBufferMappedPosition = (_currentPixelBufferMappedPosition + 1) % _pixelBuffers.size();
    currentMappedBuffer.release();
    qDebug() << "after mapping" << _currentPixelBufferReadPosition << _currentPixelBufferWritePosition << _currentPixelBufferMappedPosition;

    return frameBuffer;
}

void OpenGLPainter2::setVideoSize(const QSize &frameSize)
{
    if (!_openGLInitialized) {
        _widget->context()->makeCurrent();
        initializeOpenGL();
        qDebug() << "initializing opengl";
    }
    if (frameSize != _sourceSize) {
        _sourceSizeDirty = true;
        _sourceSize = frameSize;
        initYuv420PTextureInfo();
    }
}

void OpenGLPainter2::setFrameLoaded(int index, qint64 frameNumber, const QSize &)
{
    QTime time;
    time.start();
    _widget->context()->makeCurrent();
    _currentPixelBufferWritePosition = index;
    _pixelBuffers[index].bind();
    _pixelBuffers[index].unmap();
    _pixelBuffers[index].release();
    _frameNumbers[index] = frameNumber;
    _firstFrameLoaded = true;
    glFlush();
    qDebug() << "after loading" << _currentPixelBufferReadPosition << _currentPixelBufferWritePosition << _currentPixelBufferMappedPosition;
}

void OpenGLPainter2::requestNewFrames()
{
    while (_currentPixelBufferMappedPosition != _currentPixelBufferReadPosition) {
        emit frameNeeded(getNextFrameBuffer());
    }
}

bool OpenGLPainter2::showFrame(qint64 frameNumber)
{
    qDebug() << "showFrame" << frameNumber;
    // if we are requested to show the current frame, do nothing.
    if (_frameNumbers[_currentPixelBufferReadPosition] >= frameNumber) {
        qDebug() << "now new frame shown, because" << _frameNumbers[_currentPixelBufferReadPosition] << ">=" << frameNumber;
        return false;
    }

    quint32 newIndex = (_currentPixelBufferReadPosition + 1) % _pixelBuffers.size();
    while (_frameNumbers[newIndex] < frameNumber && newIndex != _currentPixelBufferReadPosition) {
        newIndex = (newIndex + 1) % _pixelBuffers.size();
    }
    if (newIndex == _currentPixelBufferWritePosition) {
        qDebug() << "frame not present, have to wait for new frames to arrive to catch up.";
    }

    _currentPixelBufferReadPosition = newIndex;
    requestNewFrames();
    return true;
}

void OpenGLPainter2::reset()
{
    _firstFrameLoaded = false;
}

void OpenGLPainter2::handleLoggedMessage(const QOpenGLDebugMessage &debugMessage)
{
    qDebug() << "opengl log message" << debugMessage.message();
}

/**
 * @brief compile the opengl shader program from the sources and link it, if the program is not yet linked.
 */
void OpenGLPainter2::initializeOpenGL()
{
    Q_ASSERT_X(!_program.isLinked(), "initializeOpenGL", "OpenGL already initialized");
    qDebug() << "INITIALIZING OPENGL";
    _glFunctions = QOpenGLFunctions(QOpenGLContext::currentContext());
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":///shaders/vertexshader.glsl")) {
        qFatal("Unable to add vertex shader: %s", qPrintable(_program.log()));
    }
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragmentshader.glsl")) {
        qFatal("Unable to add fragment shader: %s", qPrintable(_program.log()));
    }
    if (!_program.link()) {
        qFatal("Unable to link shader program: %s", qPrintable(_program.log()));
    }
    QOpenGLContext* glContext = QOpenGLContext::currentContext();
    if (!_glFunctions.hasOpenGLFeature(QOpenGLFunctions::NPOTTextures)) {
        qFatal("OpenGL needs to have support for 'Non power of two textures'");
    }
    if (!glContext->hasExtension("GL_ARB_pixel_buffer_object")) {
        qFatal("GL_ARB_pixel_buffer_object is missing");
    }
    if (!_glFunctions.hasOpenGLFeature(QOpenGLFunctions::Buffers)) {
        qFatal("OpenGL needs to have support for vertex buffers");
    }
    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    connect(logger, &QOpenGLDebugLogger::messageLogged, this, &OpenGLPainter2::handleLoggedMessage);
    logger->initialize();
    logger->startLogging();

    qDebug() << "generating textures.";
    glGenTextures(1, &_yTextureId);
    glGenTextures(1, &_uTextureId);
    glGenTextures(1, &_vTextureId);

    _openGLInitialized = true;
}

void OpenGLPainter2::initializeVertexCoordinatesBuffer(const QRectF& videoRect)
{
    const QVector<GLfloat> vertexCoordinates =
    {
        GLfloat(videoRect.left()), GLfloat(videoRect.top()),
        GLfloat(videoRect.right() + 1), GLfloat(videoRect.top()),
        GLfloat(videoRect.left()), GLfloat(videoRect.bottom() + 1),
        GLfloat(videoRect.right() + 1), GLfloat(videoRect.bottom() + 1)
    };
    if (_vertexBuffer.isCreated()) {
        _vertexBuffer.destroy();
    }
    _vertexBuffer.create();
    _vertexBuffer.bind();
    _vertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _vertexBuffer.allocate(vertexCoordinates.data(), sizeof(GLfloat) * vertexCoordinates.size());
    _vertexBuffer.release();
}

void OpenGLPainter2::adjustPaintAreas(const QRectF& targetRect, Qt::AspectRatioMode aspectRationMode)
{
    if (_sourceSizeDirty || targetRect != _targetRect || aspectRationMode != _aspectRatioMode) {
        _targetRect = targetRect;

        // change size of video to fit the targetRect completely.
        QSizeF sourceSizeF(_sourceSize);
        const QSizeF videoSizeAdjusted(sourceSizeF.scaled(targetRect.size(), aspectRationMode));

        QRectF videoRect = QRectF(QPointF(), videoSizeAdjusted);
        videoRect.moveCenter(targetRect.center());

        initializeVertexCoordinatesBuffer(videoRect);

        if (targetRect.left() == videoRect.left()) {
            // black bars on top and bottom
            _blackBar1 = QRectF(targetRect.topLeft(), videoRect.topRight());
            _blackBar2 = QRectF(videoRect.bottomLeft(), _targetRect.bottomRight());
        } else {
            // black bars on the sidex
            _blackBar1 = QRectF(targetRect.topLeft(), videoRect.bottomLeft());
            _blackBar2 = QRectF(videoRect.topRight(), _targetRect.bottomRight());
        }
        _sourceSizeDirty = false;
    }
}

quint32 OpenGLPainter2::combinedSizeOfTextures()
{
    quint32 size = 0u;
    for (int i = 0; i < 3; ++i) {
        size += _textureWidths[i] * _textureHeights[i];
    }
    return size;
}

void OpenGLPainter2::initializeTextureCoordinatesBuffer()
{
    const QVector<GLfloat> textureCoordinates = {
        0, 0,
        static_cast<GLfloat>(_sourceSize.width()), 0,
        0, static_cast<GLfloat>(_sourceSize.height()),
        static_cast<GLfloat>(_sourceSize.width()), static_cast<GLfloat>(_sourceSize.height())
    };

    if (_textureCoordinatesBuffer.isCreated()) {
        _textureCoordinatesBuffer.destroy();
    }
    _textureCoordinatesBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _textureCoordinatesBuffer.create();
    _textureCoordinatesBuffer.bind();
    _textureCoordinatesBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _textureCoordinatesBuffer.allocate(textureCoordinates.data(), sizeof(GLfloat) * textureCoordinates.size());
    _textureCoordinatesBuffer.release();
}

void OpenGLPainter2::initYuv420PTextureInfo()
{
    int bytesPerLine = (_sourceSize.width() + 3) & ~3;
    int bytesPerLine2 = (_sourceSize.width() / 2 + 3) & ~3;
    qDebug() << "bytes per line = " << bytesPerLine << bytesPerLine2;
    _textureWidths[0] = bytesPerLine;
    _textureHeights[0] = _sourceSize.height();
    _textureOffsets[0] = 0;
    _textureWidths[1] = bytesPerLine2;
    _textureHeights[1] = _sourceSize.height() / 2;
    _textureOffsets[1] = bytesPerLine * _sourceSize.height();
    _textureWidths[2] = bytesPerLine2;
    _textureHeights[2] = _sourceSize.height() / 2;
    _textureOffsets[2] = bytesPerLine * _sourceSize.height() + bytesPerLine2 * _sourceSize.height()/2;

    initializeTextureCoordinatesBuffer();

    for (quint32 i = 0; i < _pixelBuffers.size(); ++i) {
        if (_pixelBuffers[i].isCreated()) {
            _pixelBuffers[i].destroy();
        }
        QOpenGLBuffer pixelBuffer(QOpenGLBuffer::PixelUnpackBuffer);
        pixelBuffer.create();
        pixelBuffer.bind();
        pixelBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
        pixelBuffer.allocate(combinedSizeOfTextures());
        pixelBuffer.release();
        _pixelBuffers[i] = pixelBuffer;
    }
    _texturesInitialized = false;
}

