#include "videowidget.h"

#include <QApplication>
#include <QDateTime>
#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QThread>
#include <QtOpenGL/qgl.h>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QGLWidget(parent), _videoDecoder(new VideoDecoder),
	_playDelayTimer(new QTimer(this)),
	_playTimer(new QTimer(this)),
	_decoderThread(new QThread(this))

{
	_playDelayTimer->setSingleShot(true);
	_playDelayTimer->setInterval(250);
	connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));
	connect(_playTimer, SIGNAL(timeout()), _videoDecoder, SLOT(nextFrame()));

	_decoderThread->start();
	_videoDecoder->moveToThread(_decoderThread);

	connect(_videoDecoder, SIGNAL(frameReady(quint32)), SLOT(frameReady(quint32)));
}

VideoWidget::~VideoWidget()
{
	_decoderThread->quit();
	_decoderThread->wait();

	delete _videoDecoder;
}

void VideoWidget::playVideo()
{
	QMetaObject::invokeMethod(_videoDecoder, "openFile",
							  Q_ARG(QString, _currentRealLiveVideo.videoInformation().videoFilename()));
}


void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playTimer->isActive())
		_playTimer->stop();
	if (_playDelayTimer->isActive())
		_playDelayTimer->stop();
	_currentRealLiveVideo = rlv;

	int frameDelay = 1000 / rlv.videoInformation().frameRate();
	_playTimer->setInterval(frameDelay);
	_playDelayTimer->start();
}

void VideoWidget::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		_playTimer->stop();
		return;
	}

	if (!_currentRealLiveVideo.isValid())
		return;

	const Course& course = _currentRealLiveVideo.courses()[courseNr];
	course.start();
	_playTimer->start();
}

void VideoWidget::enterEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
}

void VideoWidget::leaveEvent(QEvent*)
{
	QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
}

void VideoWidget::frameReady(quint32)
{
	repaint();
}

void VideoWidget::paintGL()
{
	_videoDecoder->doWithImage(*this);
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

/*! This method should only be called from the video decoder. */
void VideoWidget::handleImage(const QImage &image)
{
	glEnable(GL_TEXTURE_RECTANGLE_ARB);

	if (_texture != 0) {
		deleteTexture(_texture);
	}
	_texture = bindTexture(image, GL_TEXTURE_RECTANGLE_ARB,  GL_RGBA, QGLContext::NoBindOption);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _texture);
	GLenum error;
	if( ( error = glGetError() ) != GL_NO_ERROR )
	{
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

	GLfloat width = image.width();
	GLfloat height = image.height();

	float imageRatio = width / height;
	float widgetRatio = (float) this->width() / (float) this->height();

	float clippedBorderWidth = 0;
	float clippedBorderHeight= 0;
	if (imageRatio > widgetRatio) {
		clippedBorderWidth = (width - ((widgetRatio / imageRatio) * width)) / 2;
	} else if (imageRatio < widgetRatio) {
		clippedBorderHeight = (height - ((imageRatio / widgetRatio) * height)) /2;
	}

//	qDebug() << "widget = " << this->width() << "x" << this->height();
//	qDebug() << "image = " << width << "x" << height;
//	qDebug() << "borderwidth = " << clippedBorderWidth;
//	qDebug() << "borderheight = " << clippedBorderHeight;


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


