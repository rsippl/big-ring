#ifndef OPENGLPAINTER_H
#define OPENGLPAINTER_H

#include <QObject>
#include <QtOpenGL/QGLShaderProgram>
#include <QtOpenGL/QGLWidget>

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
    void initShaders();
    void initYuv420PTextureInfo();
    void uploadTextures();
    void adjustPaintAreas(const QRectF& targetRect);

    QGLWidget* _widget;
    GstSample* _currentSample;

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
    QVector<GLfloat> _textureCoordinates;

    QGLShaderProgram _program;
};

#endif // OPENGLPAINTER_H