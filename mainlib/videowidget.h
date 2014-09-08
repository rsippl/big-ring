#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "reallifevideo.h"

#include <GL/glew.h>
#include <QtCore/QTimer>
#include <QtOpenGL/QGLWidget>
#include <QtCore/QVector>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>

#include "videodecoder.h"

class Cyclist;

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
	explicit VideoWidget(const Cyclist& cyclist, QWidget *parent = 0);
	virtual ~VideoWidget();

	bool loadFrame(Frame& frame);
	bool buffersFull() const;
	/** Display a frame */
	void displayNextFrame(quint32 frameNr);
	void clearOpenGLBuffers();

	void setRlv(RealLifeVideo rlv);
protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);

private:
	void loadPlaneTexturesFromPbo(const QString& textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, size_t offset);
	void paintFrame();
	void loadTexture();

	const QVector<GLfloat>& calculatetextureCoordinates();

	void drawProfile();

	qreal distanceToX(float distance, int pathWidth) const;
	qreal altitudeToY(float altitudeAboveMinimum, float altitudeDiff, int pathHeight) const;

	QSize _frameSize;
	int _lineSize;

	const Cyclist& _cyclist;
	QVector<GLuint> _pixelBufferObjects;
	QVector<quint32> _frameNumbers;
	GLuint _vertexBufferObject;
	GLuint _textureCoordinatesBufferObject;
	QOpenGLShaderProgram _shaderProgram;

	RealLifeVideo _currentRlv;


	quint32 _frameRate;
	quint32 _index, _nextIndex;

	QVector<GLfloat> _vertexCoordinates;
	QVector<GLfloat> _textureCoordinates;

	bool _texturesInitialized;
	QPainterPath _profilePath;
};

#endif // VIDEOWIDGET_H