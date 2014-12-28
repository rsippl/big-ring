#ifndef OPENGLPAINTER_H
#define OPENGLPAINTER_H

#include <QObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QtOpenGL/QGLWidget>
#include <QtGui/QOpenGLFunctions>

extern "C" {
#include <gst/gst.h>
}
class OpenGLPainter : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLPainter(QGLWidget* widget, QObject *parent = 0);
    virtual ~OpenGLPainter();

    void paint(QPainter* painter, const QRectF& rect);

signals:

public slots:
    void setCurrentSample(GstSample* sample);

private:
    QSizeF getSizeFromSample(GstSample* sample);
    void initializeOpenGL();
    void initYuv420PTextureInfo();
    void loadPlaneTexturesFromPbo(int glTextureUnit, int textureUnit,
                                  int lineSize, int height, size_t offset);
    void adjustPaintAreas(const QRectF& targetRect);
    quint32 combinedSizeOfTextures();

    QGLWidget* _widget;
    QOpenGLFunctions _glFunctions;
    bool _openGLInitialized;
    GstSample* _currentSample;
    bool _texturesInitialized;
    QSizeF _sourceSize;
    QRectF _targetRect;
    QRectF _videoRect;
    QRectF _blackBar1, _blackBar2;
    bool _sourceSizeDirty;

    int _textureCount;
    GLuint _yTextureId;
    GLuint _uTextureId;
    GLuint _vTextureId;

    int _textureWidths[3];
    int _textureHeights[3];
    int _textureOffsets[3];

    QOpenGLBuffer _textureCoordinatesBuffer;
    QOpenGLBuffer _vertexBuffer;
    QOpenGLBuffer _pixelBuffer;

    QOpenGLShaderProgram _program;
};

#endif // OPENGLPAINTER_H
