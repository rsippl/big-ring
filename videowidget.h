#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <GL/glew.h>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
#include <QtCore/QVector>
#include <QtGui/QOpenGLBuffer>
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

	bool loadFrame(Frame& frame);
	bool buffersFull() const;
	/** Display a frame */
	void displayNextFrame();
	void clearOpenGLBuffers();

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);

private:
	void initializeAndLoadPlaneTextureFromPbo(const QString &textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, size_t offset);
	void loadPlaneTexturesFromPbo(const QString& textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, size_t offset);
	void paintFrame();
	void loadTexture();

	const QVector<GLfloat>& calculatetextureCoordinates();

	QSize _frameSize;
	int _lineSize;

	QVector<GLuint> _pixelBufferObjects;
	GLuint _vertexBufferObject;
	GLuint _textureCoordinatesBufferObject;
	QOpenGLShaderProgram _shaderProgram;


	quint32 _frameRate;
	quint32 _index, _nextIndex;

	QVector<GLfloat> _vertexCoordinates;
	QVector<GLfloat> _textureCoordinates;

	bool _texturesInitialized;
};

#endif // VIDEOWIDGET_H
