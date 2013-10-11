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

extern "C" {
#include <libavcodec/avcodec.h>
}

VideoWidget::VideoWidget(QWidget *parent) :
	QGLWidget(parent), _pixelBufferObjects(10, 0), _vertexBufferObject(0u),
	_index(0), _nextIndex(0), _texturesInitialized(false)
{
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	setAutoBufferSwap(true);
}

VideoWidget::~VideoWidget()
{
	makeCurrent();
	glDeleteBuffersARB(_pixelBufferObjects.size(), _pixelBufferObjects.data());
	glDeleteBuffersARB(1, &_vertexBufferObject);
}

bool VideoWidget::loadFrame(Frame &frame)
{
	_lineSize = frame.avFrame->linesize[0];
	_frameSize = QSize(frame.avFrame->width, frame.avFrame->height);

	makeCurrent();
	int pboSize = _lineSize * _frameSize.height() * (1.5);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pixelBufferObjects.at(_nextIndex));
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, pboSize, 0, GL_DYNAMIC_DRAW_ARB);

	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
											GL_WRITE_ONLY_ARB);
	if(ptr)
	{
		// load all three planes
		memcpy(ptr, frame.avFrame->data[0], frame.avFrame->linesize[0] * frame.avFrame->height);
		size_t uOffset = frame.avFrame->linesize[0] * frame.avFrame->height;
		memcpy(ptr + uOffset, frame.avFrame->data[1], frame.avFrame->linesize[1] * frame.avFrame->height / 2);
		size_t vOffset = uOffset + frame.avFrame->linesize[1] * frame.avFrame->height / 2;
		memcpy(ptr + vOffset, frame.avFrame->data[2], frame.avFrame->linesize[2] * frame.avFrame->height / 2);
		glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
	}
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	_nextIndex = (_nextIndex + 1) % _pixelBufferObjects.size();
	return !((_nextIndex + 1 % _pixelBufferObjects.size()) == _index);
}

bool VideoWidget::buffersFull() const
{

//	qDebug() << "index:" << _index << "nextindex" << _nextIndex << (_nextIndex + 1) % _pixelBufferObjects.size() << _nextIndex + 1 % _pixelBufferObjects.size();
	return ((_nextIndex + 1) % _pixelBufferObjects.size()) == _index;
}



void VideoWidget::enterEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void VideoWidget::leaveEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

const QVector<GLfloat> &VideoWidget::calculatetextureCoordinates()
{
	if (_textureCoordinates.isEmpty()) {
		GLfloat width = _frameSize.width();
		GLfloat height = _frameSize.height();

		float imageRatio = width / height;
		float widgetRatio = (float) this->width() / (float) this->height();

		float clippedBorderWidth = 0;
		float clippedBorderHeight= 0;
		if (imageRatio > widgetRatio) {
			clippedBorderWidth = (width - ((widgetRatio / imageRatio) * width)) / 2;
		} else if (imageRatio < widgetRatio) {
			clippedBorderHeight = (height - ((imageRatio / widgetRatio) * height)) /2;
		}

		_textureCoordinates = {
			clippedBorderWidth, clippedBorderHeight,
			clippedBorderWidth, height - clippedBorderHeight,
			width - clippedBorderWidth, height - clippedBorderHeight,
			width - clippedBorderWidth, clippedBorderHeight
		};
		glBindBuffer(GL_ARRAY_BUFFER_ARB, _textureCoordinatesBufferObject);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(float) * _textureCoordinates.size(), _textureCoordinates.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);
	}
	return _textureCoordinates;
}

void VideoWidget::displayNextFrame()
{
	_index = (_index + 1) % _pixelBufferObjects.size();
	repaint();
}

void VideoWidget::clearOpenGLBuffers()
{
	_textureCoordinates.clear();
	_texturesInitialized = false;
	_nextIndex = 1;
	_index = 0;

	_lineSize = 0;
	_frameSize = QSize();
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
	if (!GL_ARB_vertex_buffer_object) {
		qDebug() << "This program needs the GL_ARB_vertex_buffer_objects extendsion.";
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
	_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,
										   ":///shaders/vertexshader.glsl");
	_shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,
										   ":/shaders/fragmentshader.glsl");
	if (!_shaderProgram.link()) {
		qFatal("Unable to link shader program");
	}
	if (!_shaderProgram.bind()) {
		qFatal("Unable to bind shader program");
	}
	glGenBuffersARB(1, &_vertexBufferObject);
	glGenBuffersARB(1, &_textureCoordinatesBufferObject);
}

void VideoWidget::paintFrame()
{
	calculatetextureCoordinates();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vertexBufferObject);
	glVertexPointer(2, GL_FLOAT, 0, 0);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, _textureCoordinatesBufferObject);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

}

void VideoWidget::loadTexture()
{
	size_t uTexOffset = _lineSize * _frameSize.height();
	size_t vTexOffset = uTexOffset + (_lineSize / 2) * _frameSize.height() / 2;
	if (_texturesInitialized) {
		loadPlaneTexturesFromPbo("yTex", GL_TEXTURE0, 0, _lineSize, _frameSize.height(), (size_t) 0);
		loadPlaneTexturesFromPbo("uTex", GL_TEXTURE1, 1, _lineSize / 2, _frameSize.height() / 2 , uTexOffset);
		loadPlaneTexturesFromPbo("vTex", GL_TEXTURE2, 2, _lineSize / 2, _frameSize.height() / 2, vTexOffset);
	} else {
		initializeAndLoadPlaneTextureFromPbo("yTex", GL_TEXTURE0, 0, _lineSize, _frameSize.height(), (size_t) 0);
		initializeAndLoadPlaneTextureFromPbo("uTex", GL_TEXTURE1, 1, _lineSize / 2, _frameSize.height() / 2, uTexOffset);
		initializeAndLoadPlaneTextureFromPbo("vTex", GL_TEXTURE2, 2, _lineSize / 2, _frameSize.height() / 2, vTexOffset);

		_texturesInitialized = true;
	}
}

void VideoWidget::initializeAndLoadPlaneTextureFromPbo(const QString& textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, size_t offset)
{
	glActiveTexture(glTextureUnit);
	GLint i = glGetUniformLocation(_shaderProgram.programId(), textureLocationName.toStdString().c_str());
	glUniform1i(i, textureUnit);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureUnit);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pixelBufferObjects.at(_index));
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_LUMINANCE, lineSize, height,
				 0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (void*) offset);
}

void VideoWidget::loadPlaneTexturesFromPbo(const QString& textureLocationName, int glTextureUnit, int textureUnit, int lineSize, int height, size_t offset)
{
	glActiveTexture(glTextureUnit);
	GLint i = glGetUniformLocation(_shaderProgram.programId(), textureLocationName.toStdString().c_str());
	glUniform1i(i, textureUnit);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, textureUnit);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _pixelBufferObjects.at(_index));
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, lineSize, height,
					GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*) offset);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_RECTANGLE_NV,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

}

void VideoWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!_frameSize.isValid()) {
		return;
	}
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	_shaderProgram.bind();
	loadTexture();
	paintFrame();

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

	_vertexCoordinates = {
		0, (float) this->height(),
		0, 0,
		(float) this->width(), 0,
		(float) this->width(), (float) this->height()
	};
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, _vertexBufferObject);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, _vertexCoordinates.size() * sizeof(float), _vertexCoordinates.data(), GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	_textureCoordinates.clear();
}
