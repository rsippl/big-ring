#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <GL/glew.h>
#include <QTimer>
#include <QGLWidget>
#include <QVector>

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
public slots:
	void setFrameRate(quint32 frameRate);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int w, int h);

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);

private:
	quint32 _currentFrameNumber;
	Frame _currentFrame;
	GLuint _texture;

	QVector<GLuint> _pixelBufferObjects;

	quint32 _frameRate;
	quint32 _index, _nextIndex;
};

#endif // VIDEOWIDGET_H
