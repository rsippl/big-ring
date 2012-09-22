#include "videowidget.h"

#include <QMetaObject>
#include <QtDebug>
#include <QResizeEvent>
#include <QAbstractAnimation>
#include <QLabel>
#include <QVBoxLayout>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QWidget(parent),
	_mediaPlayer(new QMediaPlayer(this))
{

	setAutoFillBackground(false);
//	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, Qt::black);
	setPalette(palette);

	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	QVBoxLayout *layout = new QVBoxLayout(this);

	_videoWidget = new QVideoWidget(this);
	_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
	_mediaPlayer->setVideoOutput(_videoWidget);

	connect(_mediaPlayer, SIGNAL(durationChanged(qint64)),
			this, SIGNAL(videoDurationAvailable(qint64)));

	layout->addWidget(_videoWidget);

	_speedLabel = new QLabel("0 km/h", this);
	layout->addWidget(_speedLabel);
	_distanceLabel = new QLabel("0 m", this);
	layout->addWidget(_distanceLabel);
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

void VideoWidget::setSpeed(float speed)
{
	_speedLabel->setText(QString("%1 km/h").arg(speed));
}

void VideoWidget::setDistance(float distance)
{
	_distanceLabel->setText(QString("%1 m").arg(distance));
}
