#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H
#include <../glew-1.9.0/include/GL/glew.h>
#include <QTimer>
#include <QGLWidget>

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
	void displayFrame(quint32 frameNr, QImage &imageFrame);

protected:
	virtual void paintGL();
	virtual void resizeGL(int w, int h);

	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);

private:
	quint32 _currentFrameNumber;
	QImage _currentFrame;
	GLuint _texture;
};

#endif // VIDEOWIDGET_H
