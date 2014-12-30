#include "openglpainter.h"

#include <QtGui/QOpenGLContext>
#include <QtCore/QtMath>
#include <QtCore/QTime>
extern "C" {
#include <gst/video/video-info.h>
}

OpenGLPainter::OpenGLPainter(QGLWidget* widget, QObject *parent) :
    QObject(parent), _widget(widget), _openGLInitialized(false), _firstFrameLoaded(false),
    _texturesInitialized(false), _aspectRatioMode(Qt::KeepAspectRatioByExpanding)
{
    Q_INIT_RESOURCE(shaders);
}

OpenGLPainter::~OpenGLPainter()
{
    // empty
}

void OpenGLPainter::loadTextures()
{
    loadPlaneTexturesFromPbo(GL_TEXTURE0, _yTextureId, _textureWidths[0], _textureHeights[0], (size_t) 0);
    loadPlaneTexturesFromPbo(GL_TEXTURE1, _uTextureId, _textureWidths[1], _textureHeights[1], _textureOffsets[1]);
    loadPlaneTexturesFromPbo(GL_TEXTURE2, _vTextureId, _textureWidths[2], _textureHeights[2], _textureOffsets[2]);

    _texturesInitialized = true;
}

void OpenGLPainter::loadPlaneTexturesFromPbo(int glTextureUnit, int textureUnit,
                                           int lineSize, int height, size_t offset)
{
    glActiveTexture(glTextureUnit);
    glBindTexture(GL_TEXTURE_RECTANGLE, textureUnit);

    _pixelBuffer.bind();

    if (_texturesInitialized) {
        glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, lineSize, height,
                        GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*) offset);
    } else {
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_LUMINANCE, lineSize, height,
                     0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (void*) offset);
    }

    _pixelBuffer.release();
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
}

void OpenGLPainter::paint(QPainter *painter, const QRectF &rect, Qt::AspectRatioMode aspectRatioMode)
{
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
}



void OpenGLPainter::setCurrentSample(GstSample *sample)
{
    if (!_openGLInitialized) {
        _widget->context()->makeCurrent();
        initializeOpenGL();
    }
    if (getSizeFromSample(sample) != _sourceSize) {
        _sourceSizeDirty = true;
        _sourceSize = getSizeFromSample(sample);
        initYuv420PTextureInfo();
    }

    _pixelBuffer.bind();
    void* ptr = _pixelBuffer.map(QOpenGLBuffer::WriteOnly);
    if(ptr) {
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo mapInfo;
        if (gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
            // load all three planes in one operation
            memcpy(ptr, mapInfo.data, combinedSizeOfTextures());
            gst_buffer_unmap(buffer, &mapInfo);
        }
        _pixelBuffer.unmap();
    }
    _pixelBuffer.release();
    gst_sample_unref(sample);
    _firstFrameLoaded = true;
}


QSizeF OpenGLPainter::getSizeFromSample(GstSample* sample)
{
    const GstCaps* caps = gst_sample_get_caps(sample);
    GstVideoInfo videoInfo;
    gst_video_info_from_caps(&videoInfo, caps);
    const GstStructure* structure = gst_caps_get_structure(caps, 0);
    gint width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    return QSizeF(width, height);
}

/**
 * @brief compile the opengl shader program from the sources and link it, if the program is not yet linked.
 */
void OpenGLPainter::initializeOpenGL()
{
    Q_ASSERT_X(!_program.isLinked(), "initializeOpenGL", "OpenGL already initialized");

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

    qDebug() << "generating textures.";
    glGenTextures(1, &_yTextureId);
    glGenTextures(1, &_uTextureId);
    glGenTextures(1, &_vTextureId);

    _openGLInitialized = true;
}

void OpenGLPainter::initializeVertexCoordinatesBuffer(const QRectF& videoRect)
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

void OpenGLPainter::adjustPaintAreas(const QRectF& targetRect, Qt::AspectRatioMode aspectRationMode)
{
    if (_sourceSizeDirty || targetRect != _targetRect || aspectRationMode != _aspectRatioMode) {
        _targetRect = targetRect;

        // change size of video to fit the targetRect completely.
        const QSizeF videoSizeAdjusted = _sourceSize.scaled(targetRect.size(), aspectRationMode);

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

quint32 OpenGLPainter::combinedSizeOfTextures()
{
    quint32 size = 0u;
    for (int i = 0; i < 3; ++i) {
        size += _textureWidths[i] * _textureHeights[i];
    }
    return size;
}

void OpenGLPainter::initializeTextureCoordinatesBuffer()
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

void OpenGLPainter::initYuv420PTextureInfo()
{
    int bytesPerLine = (_sourceSize.toSize().width() + 3) & ~3;
    int bytesPerLine2 = (_sourceSize.toSize().  width() / 2 + 3) & ~3;
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

    if (_pixelBuffer.isCreated()) {
        _pixelBuffer.destroy();
    }
    _pixelBuffer = QOpenGLBuffer(QOpenGLBuffer::PixelUnpackBuffer);
    _pixelBuffer.create();
    _pixelBuffer.bind();
    _pixelBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _pixelBuffer.allocate(combinedSizeOfTextures());
    _pixelBuffer.release();

    _texturesInitialized = false;
}

