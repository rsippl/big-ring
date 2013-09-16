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
	QGLWidget(parent), _texture(0), _pixelBufferObjects(2, 0),
	_index(0), _nextIndex(1)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setAutoBufferSwap(true);
}

VideoWidget::~VideoWidget()
{
	makeCurrent();
	if (_texture) {
		glDeleteTextures(1, &_texture);
	}
	glDeleteBuffersARB(_pixelBufferObjects.size(), _pixelBufferObjects.data());
}



void VideoWidget::enterEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void VideoWidget::leaveEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void VideoWidget::displayFrame(Frame& frame)
{
	if (frame.frameNr != UNKNOWN_FRAME_NR) {
		_currentFrameNumber = frame.frameNr;
		_currentFrame = frame;
		repaint();
	}
}

void VideoWidget::clearOpenGLBuffers()
{
	makeCurrent();
	if (_texture) {
		glDeleteTextures(1, &_texture);
		_texture = 0;
	}
}

void VideoWidget::setFrameRate(quint32 frameRate)
{
	_frameRate = frameRate;
}

void VideoWidget::initializeGL()
{
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
	if (!GL_ARB_pixel_buffer_object) {
		qDebug() << "This program needs the GL_ARB_pixel_buffer_object extension, but it seems to be disabled.";
		qFatal("exiting..");
	}
	glViewport (0, 0, width(), height());
	glMatrixMode (GL_PROJECTION);
	glDisable(GL_DEPTH_TEST);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glOrtho(0, width(),0,height(),-1,1);
	glMatrixMode (GL_MODELVIEW);

	glGenBuffersARB(_pixelBufferObjects.size(), _pixelBufferObjects.data());
	_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
										   "#version 120\n"
										   "void main(void)\n"
										   "{\n"
										   "   gl_TexCoord[0] = gl_MultiTexCoord0;\n"
										   "   gl_Position = ftransform();\n"
										   "}");
	_shaderProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
										   "#version 120\n"
										   "uniform sampler2DRect texture;\n"
										   "void main(void)\n"
										   "{\n"
										   "   gl_FragColor = texture2DRect(texture, gl_TexCoord[0].st);\n"
										   "}");
	if (!_shaderProgram.link()) {
		qFatal("Unable to link shader program");
	}
	if (!_shaderProgram.bind()) {
		qFatal("Unable to bind shader program");
	}
}

void VideoWidget::paintFrame()
{
	glActiveTexture(_texture);
	GLfloat width = _currentFrame.width;
	GLfloat height = _currentFrame.height;

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
	glTexCoord2f(clippedBorderWidth, clippedBorderHeight);
	glVertex2f(0,this->height());
	glTexCoord2f(clippedBorderWidth, height - clippedBorderHeight);
	glVertex2f(0,0);
	glTexCoord2f(width - clippedBorderWidth, height - clippedBorderHeight);
	glVertex2f(this->width(), 0);
	glTexCoord2f(width - clippedBorderWidth, clippedBorderHeight);
	glVertex2f(this->width(), this->height());
	glEnd();
}

void VideoWidget::loadNextFrameToPixelBuffer()
{
	_nextIndex = (_nextIndex + 1) % _pixelBufferObjects.size();
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pixelBufferObjects.at(_nextIndex));
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, _currentFrame.numBytes, 0, GL_STREAM_DRAW_ARB);

	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
											GL_WRITE_ONLY_ARB);
	if(ptr)
	{
		memcpy(ptr, _currentFrame.data.data(), _currentFrame.numBytes);
		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	}
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

void VideoWidget::loadTexture()
{
	if (_texture) {
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _texture);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pixelBufferObjects.at(_index));
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, _currentFrame.width, _currentFrame.height,
						GL_BGRA, GL_UNSIGNED_BYTE, 0);
	} else {
		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _texture);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, _currentFrame.width, _currentFrame.height,
					 0, GL_BGRA, GL_UNSIGNED_BYTE, _currentFrame.data.data());
	}
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
}

void VideoWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_index = (_index + 1) % _pixelBufferObjects.size();

	if (_currentFrame.data.isNull()) {
		return;
	}
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	_shaderProgram.bind();
	loadTexture();
	paintFrame();
	loadNextFrameToPixelBuffer();

	// unbind texture
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
