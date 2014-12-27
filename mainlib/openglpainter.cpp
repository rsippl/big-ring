#include "openglpainter.h"

#include <QtCore/QtMath>
extern "C" {
#include <gst/video/video-info.h>
}

namespace {
const QString FRAGMENT_SHADER =
        "#version 120\n"
        "uniform sampler2DRect texY;\n"
        "uniform sampler2DRect texU, texV;\n"
        "varying highp vec2 textureCoord;\n"
        "void main(void)\n"
        "{\n"
        "         float nx, ny, y, u, v, r, g, b;\n"
        "\n"
        "         nx = gl_TexCoord[0].x;\n"
        "         ny = gl_TexCoord[0].y;\n"
        "\n"
        "         y = texture2DRect(texY, vec2(nx, ny)).r;\n"
        "         u = texture2DRect(texU, vec2(nx * 0.5, ny * 0.5)).r;\n"
        "         v = texture2DRect(texV, vec2(nx * 0.5, ny * 0.5)).r;\n"
        "\n"
        "         y=1.1643*(y-0.0625);\n"
        "         u=u-0.5;\n"
        "         v=v-0.5;\n"
        "\n"
        "         r=y+1.5958*v;\n"
        "         g=y-0.39173*u-0.81290*v;\n"
        "         b=y+2.017*u;\n"
        "\n"
        "         gl_FragColor = vec4(r, g, b, 1.0);\n"
        "}\n";

//const QString FRAGMENT_SHADER =
//        "uniform sampler2D texY;\n"
//        "uniform sampler2D texU;\n"
//        "uniform sampler2D texV;\n"
//        "uniform mediump mat4 colorMatrix;\n"
//        "varying highp vec2 textureCoord;\n"
//        "void main(void)\n"
//        "{\n"
//        "    highp vec4 color = vec4(\n"
//        "           texture2D(texY, textureCoord.st).r,\n"
//        "           texture2D(texU, textureCoord.st).r,\n"
//        "           texture2D(texV, textureCoord.st).r,\n"
//        "           1.0);\n"
//        "    gl_FragColor = colorMatrix * color;\n"
//        "}\n";

const QString VERTEX_SHADER =
        "attribute highp vec4 vertexCoordArray;\n"
        "attribute highp vec2 textureCoordArray;\n"
        "uniform highp mat4 positionMatrix;\n"
        "varying highp vec2 textureCoord;\n"
        "void main(void)\n"
        "{\n"
//        "   gl_Position = positionMatrix * vertexCoordArray;\n"
                "   gl_Position = ftransform();\n"
        "   gl_TexCoord[0]=gl_MultiTexCoord0;\n"
//        "   textureCoord = textureCoordArray;\n"
        "}\n";

}

OpenGLPainter::OpenGLPainter(QGLWidget* widget, QObject *parent) :
    QObject(parent), _widget(widget), _currentSample(nullptr), _colorMatrix(updateColors())
{
}

OpenGLPainter::~OpenGLPainter()
{
    if (_currentSample) {
        gst_sample_unref(_currentSample);
    }
}

void OpenGLPainter::paint(QPainter *painter, const QRectF &rect)
{
    if (_currentSample) {
        adjustPaintAreas(rect);
        painter->fillRect(_blackBar1, Qt::black);
        painter->fillRect(_blackBar2, Qt::black);
//        painter->fillRect(_videoRect, Qt::blue);
    } else {
        painter->fillRect(rect, Qt::black);
        return;
    }
    GstBuffer* buffer = gst_sample_get_buffer(_currentSample);
    // if these are enabled, we need to reenable them after beginNativePainting()
    // has been called, as they may get disabled
    bool stencilTestEnabled = glIsEnabled(GL_STENCIL_TEST);
    bool scissorTestEnabled = glIsEnabled(GL_SCISSOR_TEST);

    painter->beginNativePainting();

    if (stencilTestEnabled)
        glEnable(GL_STENCIL_TEST);
    if (scissorTestEnabled)
        glEnable(GL_SCISSOR_TEST);

    const GLfloat vertexCoordArray[] = { GLfloat(_videoRect.left()), GLfloat(_videoRect.bottom() + 1),
                                         GLfloat(_videoRect.right() + 1), GLfloat(_videoRect.bottom() + 1),
                                         GLfloat(_videoRect.left()), GLfloat(_videoRect.top()),
                                         GLfloat(_videoRect.right() + 1), GLfloat(_videoRect.top())
                                       };
    const GLfloat textureCoordArray[] =
    {
        GLfloat(0), GLfloat(_sourceSize.height()),
        GLfloat(_sourceSize.width()), GLfloat(_sourceSize.height()),
        GLfloat(0), GLfloat(0),
        GLfloat(_sourceSize.width()), GLfloat(_sourceSize.height())
    };

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
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_LUMINANCE,
                    _textureWidths[i],
                    _textureHeights[i],
                    0,
                    GL_LUMINANCE,
                    GL_UNSIGNED_BYTE,
                    mapInfo.data + _textureOffsets[i]);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        gst_buffer_unmap(buffer, &mapInfo);
    }

    const int deviceWidth = painter->device()->width();
    const int deviceHeight = painter->device()->height();

    const QTransform transform = painter->deviceTransform();

    const GLfloat wfactor = 2.0 / deviceWidth;
    const GLfloat hfactor = -2.0 / deviceHeight;

    const GLfloat positionMatrix[4][4] =
    {
        {
            /*(0,0)*/ GLfloat(wfactor * transform.m11() - transform.m13()),
            /*(0,1)*/ GLfloat(hfactor * transform.m12() + transform.m13()),
            /*(0,2)*/ 0.0,
            /*(0,3)*/ GLfloat(transform.m13())
        }, {
            /*(1,0)*/ GLfloat(wfactor * transform.m21() - transform.m23()),
            /*(1,1)*/ GLfloat(hfactor * transform.m22() + transform.m23()),
            /*(1,2)*/ 0.0,
            /*(1,3)*/ GLfloat(transform.m23())
        }, {
            /*(2,0)*/ 0.0,
            /*(2,1)*/ 0.0,
            /*(2,2)*/ -1.0,
            /*(2,3)*/ 0.0
        }, {
            /*(3,0)*/ GLfloat(wfactor * transform.dx() - transform.m33()),
            /*(3,1)*/ GLfloat(hfactor * transform.dy() + transform.m33()),
            /*(3,2)*/ 0.0,
            /*(3,3)*/ GLfloat(transform.m33())
        }
    };

    qDebug() << "position matrix" << positionMatrix[0][0];
    _program.bind();

    glVertexPointer(2, GL_FLOAT, 0, vertexCoordArray);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _yTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _uTextureId);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _vTextureId);
    glActiveTexture(GL_TEXTURE0);

    _program.setUniformValue("texY", 0);
    _program.setUniformValue("texU", 1);
    _program.setUniformValue("texV", 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    _program.release();

    qDebug() << _program.log();
    painter->endNativePainting();
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
        if (!_program.addShaderFromSourceCode(QGLShader::Vertex, VERTEX_SHADER)) {
            qFatal("Unable to add vertex shader: %s", qPrintable(_program.log()));
        }
        if (!_program.addShaderFromSourceCode(QGLShader::Fragment, FRAGMENT_SHADER)) {
            qFatal("Unable to add fragment shader: %s", qPrintable(_program.log()));
        }
        if (!_program.link()) {
            qFatal("Unable to link shader program: %s", qPrintable(_program.log()));
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

    for (int i = 0; i < 3; i++) {
    qDebug() << "init texture" << i << "widthxheigh=" << _textureWidths[i] << _textureHeights[i];
    }
}


QMatrix4x4* OpenGLPainter::updateColors()
{
    int brightness = 0;
    int contrast = 0;
    int hue = 0;
    int saturation = 0;

    const qreal b = brightness / 200.0;
    const qreal c = contrast / 100.0 + 1.0;
    const qreal h = hue / 100.0;
    const qreal s = saturation / 100.0 + 1.0;

    const qreal cosH = qCos(M_PI * h);
    const qreal sinH = qSin(M_PI * h);

    const qreal h11 =  0.787 * cosH - 0.213 * sinH + 0.213;
    const qreal h21 = -0.213 * cosH + 0.143 * sinH + 0.213;
    const qreal h31 = -0.213 * cosH - 0.787 * sinH + 0.213;

    const qreal h12 = -0.715 * cosH - 0.715 * sinH + 0.715;
    const qreal h22 =  0.285 * cosH + 0.140 * sinH + 0.715;
    const qreal h32 = -0.715 * cosH + 0.715 * sinH + 0.715;

    const qreal h13 = -0.072 * cosH + 0.928 * sinH + 0.072;
    const qreal h23 = -0.072 * cosH - 0.283 * sinH + 0.072;
    const qreal h33 =  0.928 * cosH + 0.072 * sinH + 0.072;

    const qreal sr = (1.0 - s) * 0.3086;
    const qreal sg = (1.0 - s) * 0.6094;
    const qreal sb = (1.0 - s) * 0.0820;

    const qreal sr_s = sr + s;
    const qreal sg_s = sg + s;
    const qreal sb_s = sr + s;

    const float m4 = (s + sr + sg + sb) * (0.5 - 0.5 * c + b);

    QMatrix4x4* colorMatrix = new QMatrix4x4;
    ((*colorMatrix))(0, 0) = c * (sr_s * h11 + sg * h21 + sb * h31);
    ((*colorMatrix))(0, 1) = c * (sr_s * h12 + sg * h22 + sb * h32);
    (*colorMatrix)(0, 2) = c * (sr_s * h13 + sg * h23 + sb * h33);
    (*colorMatrix)(0, 3) = m4;

    (*colorMatrix)(1, 0) = c * (sr * h11 + sg_s * h21 + sb * h31);
    (*colorMatrix)(1, 1) = c * (sr * h12 + sg_s * h22 + sb * h32);
    (*colorMatrix)(1, 2) = c * (sr * h13 + sg_s * h23 + sb * h33);
    (*colorMatrix)(1, 3) = m4;

    (*colorMatrix)(2, 0) = c * (sr * h11 + sg * h21 + sb_s * h31);
    (*colorMatrix)(2, 1) = c * (sr * h12 + sg * h22 + sb_s * h32);
    (*colorMatrix)(2, 2) = c * (sr * h13 + sg * h23 + sb_s * h33);
    (*colorMatrix)(2, 3) = m4;

    (*colorMatrix)(3, 0) = 0.0;
    (*colorMatrix)(3, 1) = 0.0;
    (*colorMatrix)(3, 2) = 0.0;
    (*colorMatrix)(3, 3) = 1.0;

    (*colorMatrix) *= QMatrix4x4(
                1.164,  0.000,  1.596, -0.8708,
                1.164, -0.392, -0.813,  0.5296,
                1.164,  2.017,  0.000, -1.081,
                0.0,    0.000,  0.000,  1.0000);

    return colorMatrix;
}

