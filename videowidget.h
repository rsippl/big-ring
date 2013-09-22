#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <GL/glew.h>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
#include <QtCore/QVector>
#include <QtGui/QOpenGLShaderProgram>

#include "videodecoder.h"
/**
 * @brief Widget which is used to display frames from a movie file.
 *
 * The widget will take up the maximum amount of space that it can.
 * Displaying is done using the displayFrame function.
 */
class VideoWidget : public QGLWidget
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget *parent = 0);
	virtual ~VideoWidget();

	/** Display a frame */
	void displayFrame(Frame &frame);
	void clearOpenGLBuffers();
	void paintFrame();
	void loadNextFrameToPixelBuffer();
	void loadTexture();

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);

private:
	void loadPlaneTexture(const QString &textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, QSharedPointer<quint8> &data);

	const QVector<GLfloat>& calculatetextureCoordinates();

	quint32 _currentFrameNumber;
	Frame _currentFrame;

	QVector<GLuint> _pixelBufferObjects;
	QOpenGLShaderProgram _shaderProgram;

	quint32 _frameRate;
	quint32 _index, _nextIndex;

	QVector<GLfloat> _vertexCoordinates;
	QVector<GLfloat> _textureCoordinates;
};

#endif // VIDEOWIDGET_H
