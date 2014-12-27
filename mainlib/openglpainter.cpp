#include "openglpainter.h"

#include <QtGui/QOpenGLContext>
#include <QtCore/QtMath>
extern "C" {
#include <gst/video/video-info.h>
}

OpenGLPainter::OpenGLPainter(QGLWidget* widget, QObject *parent) :
    QObject(parent), _widget(widget), _currentSample(nullptr)
{
    Q_INIT_RESOURCE(shaders);
}

OpenGLPainter::~OpenGLPainter()
{
    if (_currentSample) {
        gst_sample_unref(_currentSample);
    }
}

void OpenGLPainter::uploadTextures()
{
    GstBuffer* buffer = gst_sample_get_buffer(_currentSample);
    GstMapInfo mapInfo;
    if (gst_buffer_map(buffer, &mapInfo, GST_MAP_READ)) {
        for (int i = 0; i < _textureCount; ++i) {
            qDebug() << "uploading texture" << i << "widthxheigh=" << _textureWidths[i] << _textureHeights[i];
            GLuint textureId;
            if (i == 0) {
                textureId = _yTextureId;
            } else if (i == 1) {
                textureId = _uTextureId;
            } else {
                textureId = _vTextureId;
            }
            glBindTexture(GL_TEXTURE_RECTANGLE, textureId);
            glTexImage2D(
                    GL_TEXTURE_RECTANGLE,
                    0,
                    GL_LUMINANCE,
                    _textureWidths[i],
                    _textureHeights[i],
                    0,
                    GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    mapInfo.data + _textureOffsets[i]);
            glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        gst_buffer_unmap(buffer, &mapInfo);
    }
}

void OpenGLPainter::paint(QPainter *painter, const QRectF &rect)
{
    if (!_currentSample) {
        painter->fillRect(rect, Qt::black);
        return;
    }

    adjustPaintAreas(rect);

    // if these are enabled, we need to reenable them after beginNativePainting()
    // has been called, as they may get disabled
    bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

    painter->beginNativePainting();

    if (stencilTestEnabled)
        glEnable(GL_STENCIL_TEST);
    if (scissorTestEnabled)
        glEnable(GL_SCISSOR_TEST);

    const GLfloat vertexCoordArray[] = { GLfloat(_videoRect.left()), GLfloat(_videoRect.top()),
                                         GLfloat(_videoRect.right() + 1), GLfloat(_videoRect.top()),
                                         GLfloat(_videoRect.left()), GLfloat(_videoRect.bottom() + 1),
                                         GLfloat(_videoRect.right() + 1), GLfloat(_videoRect.bottom() + 1)
                                       };
    uploadTextures();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    _program.bind();
    glTexCoordPointer(2, GL_FLOAT, 0, _textureCoordinates.data());
    glVertexPointer(2, GL_FLOAT, 0, vertexCoordArray);
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
    if (_currentSample) {
        gst_sample_unref(_currentSample);
    }
    _currentSample = sample;
    QSizeF sampleSize = getSizeFromSample(sample);
    if (sampleSize != _sourceSize) {
        _sourceSizeDirty = true;
        _sourceSize = sampleSize;
        initYuv420PTextureInfo();
        initShaders();
    }
}

QSizeF OpenGLPainter::getSizeFromSample(GstSample* sample)
{
    GstCaps* caps = gst_sample_get_caps(sample);
    GstVideoInfo videoInfo;
    gst_video_info_from_caps(&videoInfo, caps);
    GstStructure* structure = gst_caps_get_structure(caps, 0);
    gint width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    return QSizeF(width, height);
}

/**
 * @brief compile the opengl shader program from the sources and link it, if the program is not yet linked.
 */
void OpenGLPainter::initShaders()
{
    if (!_program.isLinked()) {
        if (!_program.addShaderFromSourceFile(QGLShader::Vertex, ":///shaders/vertexshader.glsl")) {
            qFatal("Unable to add vertex shader: %s", qPrintable(_program.log()));
        }
        if (!_program.addShaderFromSourceFile(QGLShader::Fragment, ":/shaders/fragmentshader.glsl")) {
            qFatal("Unable to add fragment shader: %s", qPrintable(_program.log()));
        }
        if (!_program.link()) {
            qFatal("Unable to link shader program: %s", qPrintable(_program.log()));
        }
        for (auto extension: QOpenGLContext::currentContext()->extensions()) {
            qDebug() << "extension" << extension;
        }
        QOpenGLContext* glContext = QOpenGLContext::currentContext();
        if (!glContext->hasExtension("GL_ARB_texture_rectangle")) {
            qFatal("GL_ARB_texture_rectangle is missing");
        }
        if (!glContext->hasExtension("GL_ARB_pixel_buffer_object")) {
            qFatal("GL_ARB_pixel_buffer_object is missing");
        }
        if (!glContext->hasExtension("GL_ARB_vertex_buffer_object")) {
            qFatal("GL_ARB_vertex_buffer_object");
        }

        qDebug() << "generating textures.";
        glGenTextures(1, &_yTextureId);
        glGenTextures(1, &_uTextureId);
        glGenTextures(1, &_vTextureId);
        qDebug() << "generating textures." << _yTextureId;

    }
}

void OpenGLPainter::adjustPaintAreas(const QRectF& targetRect)
{
    if (_sourceSizeDirty || targetRect != _targetRect) {
        _targetRect = targetRect;
        QSizeF videoSizeAdjusted = QSizeF(_sourceSize.width(), _sourceSize.height()).scaled(targetRect.size(), Qt::KeepAspectRatio);

        _videoRect = QRectF(QPointF(), videoSizeAdjusted);
        _videoRect.moveCenter(targetRect.center());

        if (targetRect.left() == _videoRect.left()) {
            // black bars on top and bottom
            _blackBar1 = QRectF(targetRect.topLeft(), _videoRect.topRight());
            _blackBar2 = QRectF(_videoRect.bottomLeft(), _targetRect.bottomRight());
        } else {
            // black bars on the sidex
            _blackBar1 = QRectF(targetRect.topLeft(), _videoRect.bottomLeft());
            _blackBar2 = QRectF(_videoRect.topRight(), _targetRect.bottomRight());
        }
        _sourceSizeDirty = false;
    }
}

void OpenGLPainter::initYuv420PTextureInfo()
{
    int bytesPerLine = (_sourceSize.toSize().width() + 3) & ~3;
    int bytesPerLine2 = (_sourceSize.toSize().  width() / 2 + 3) & ~3;
    qDebug() << "bytes per line = " << bytesPerLine << bytesPerLine2;
    _textureCount = 3;
    _textureWidths[0] = bytesPerLine;
    _textureHeights[0] = _sourceSize.height();
    _textureOffsets[0] = 0;
    _textureWidths[1] = bytesPerLine2;
    _textureHeights[1] = _sourceSize.height() / 2;
    _textureOffsets[1] = bytesPerLine * _sourceSize.height();
    _textureWidths[2] = bytesPerLine2;
    _textureHeights[2] = _sourceSize.height() / 2;
    _textureOffsets[2] = bytesPerLine * _sourceSize.height() + bytesPerLine2 * _sourceSize.height()/2;

    _textureCoordinates = {
        0, 0,
        static_cast<GLfloat>(_sourceSize.width()), 0,
        0, static_cast<GLfloat>(_sourceSize.height()),
        static_cast<GLfloat>(_sourceSize.width()), static_cast<GLfloat>(_sourceSize.height())
    };

    for (int i = 0; i < 3; i++) {
    qDebug() << "init texture" << i << "widthxheigh=" << _textureWidths[i] << _textureHeights[i];
    }
}

