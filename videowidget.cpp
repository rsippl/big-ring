#include "videowidget.h"

#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QGridLayout>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QWidget(parent),
	_mediaPlayer(new QMediaPlayer(this))
{

	setAutoFillBackground(false);
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, Qt::black);
	setPalette(palette);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


	QGridLayout *grid = new QGridLayout(this);
	grid->setMargin(0);

	_videoWidget = new QVideoWidget(this);
	_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
	_mediaPlayer->setVideoOutput(_videoWidget);

	connect(_mediaPlayer, SIGNAL(durationChanged(qint64)),
			this, SIGNAL(videoDurationAvailable(qint64)));

	grid->addWidget(_videoWidget, 0, 0, 3, 1);
}

VideoWidget::~VideoWidget()
{

}

void VideoWidget::loadVideo(const QString& videoFilename)
{
	_mediaPlayer->setMedia(QUrl::fromLocalFile(videoFilename));
}

void VideoWidget::playVideo()
{
	_mediaPlayer->play();
}

qint64 VideoWidget::videoDuration() const
{
	return _mediaPlayer->duration();
}

void VideoWidget::setPosition(qint64 position)
{
	_mediaPlayer->setPosition(position);
}

void VideoWidget::setRate(float rate)
{
	_mediaPlayer->setPlaybackRate(rate);
}


