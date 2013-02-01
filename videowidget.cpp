#include "videowidget.h"

#include <QApplication>
#include <QDateTime>
#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QThread>
#include <QtOpenGL/qgl.h>
#include <QAbstractAnimation>
#include <QLabel>
#include <QVBoxLayout>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QGLWidget(parent), _texture(0)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

VideoWidget::~VideoWidget()
{
}



void VideoWidget::enterEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void VideoWidget::leaveEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void VideoWidget::displayFrame(quint32 frameNr, QImage &image)
{
	if (frameNr != UNKNOWN_FRAME_NR) {
		_currentFrameNumber = frameNr;
		_currentFrame = image;
		repaint();
	}
}

void VideoWidget::initializeGL()
{
#ifndef Q_OS_WIN
	qDebug() << "Running glew init";
	GLenum error = glewInit();
	if (GLEW_OK != error) {
	  /* Problem: glewInit failed, something is seriously wrong. */
	  qDebug() << "Error:" << glewGetErrorString(error);
	  qFatal("Exiting.");
	}
	if (!GL_ARB_texture_rectangle) {
		qDebug() << "This program needs the GL_ARB_texture_rectangle extension, but it seems to be disabled.";
		qFatal("exiting..");
	}
#endif
}

void VideoWidget::paintGL()
{
	if (_currentFrame.isNull())
		return;

	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	if (_texture != 0) {
		deleteTexture(_texture);
	}
	_texture = bindTexture(_currentFrame, GL_TEXTURE_RECTANGLE_ARB,  GL_RGBA, QGLContext::NoBindOption);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _texture);
	GLenum error;
	if( ( error = glGetError() ) != GL_NO_ERROR ) {
		QString errorstring;
		switch( error ){
		case GL_INVALID_ENUM:
			errorstring = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errorstring = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			errorstring = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			errorstring = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			errorstring = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			errorstring = "GL_OUT_OF_MEMORY";
			break;
		default:
			errorstring = "UNKNOWN";
			break;
		}
		qDebug ("failed to bind texture %d %s",error,errorstring.toAscii().data());
		return;
	}
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	GLfloat width = _currentFrame.width();
	GLfloat height = _currentFrame.height();

	float imageRatio = width / height;
	float widgetRatio = (float) this->width() / (float) this->height();

	float clippedBorderWidth = 0;
	float clippedBorderHeight= 0;
	if (imageRatio > widgetRatio) {
		clippedBorderWidth = (width - ((widgetRatio / imageRatio) * width)) / 2;
	} else if (imageRatio < widgetRatio) {
		clippedBorderHeight = (height - ((imageRatio / widgetRatio) * height)) /2;
	}

	glBegin(GL_QUADS);
	glTexCoord2f(clippedBorderWidth, height - clippedBorderHeight);
	glVertex2f( 0.0f, 0.0f );
	glTexCoord2f(width - clippedBorderWidth, height - clippedBorderHeight);
	glVertex2f( this->width(), 0.0f );
	glTexCoord2f(width - clippedBorderWidth, clippedBorderHeight);
	glVertex2f( this->width(), this->height());
	glTexCoord2f(clippedBorderWidth, clippedBorderHeight);
	glVertex2f( 0.0f, this->height() );
	glEnd();

	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
}

void VideoWidget::resizeGL(int w, int h)
{
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glOrtho(0, w,0,h,-1,1);
	glMatrixMode (GL_MODELVIEW);
}
