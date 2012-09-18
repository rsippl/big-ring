#include "videowidget.h"

#include <QMetaObject>
#include <QMediaPlaylist>
#include <QtDebug>
#include <QResizeEvent>
#include <QGridLayout>
#include "videodecoder.h"

VideoWidget::VideoWidget(QWidget *parent) :
	QWidget(parent),
	_playDelayTimer(new QTimer(this))
{
	_playDelayTimer->setSingleShot(true);
	_playDelayTimer->setInterval(250);
	connect(_playDelayTimer, SIGNAL(timeout()), SLOT(playVideo()));

	setAutoFillBackground(false);
//	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_PaintOnScreen, true);

	QPalette palette = this->palette();
	palette.setColor(QPalette::Background, Qt::black);
	setPalette(palette);

	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


	QGridLayout *grid = new QGridLayout(this);
	grid->setMargin(0);

	_mediaPlayer = new QMediaPlayer(this);
	_mediaPlayer->setPlaylist(new QMediaPlaylist(_mediaPlayer));

	_videoWidget = new QVideoWidget(this);
	_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
	_mediaPlayer->setVideoOutput(_videoWidget);
	connect(_mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
			this, SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));

	grid->addWidget(_videoWidget, 0, 0, 3, 1);
}

VideoWidget::~VideoWidget()
{

}

void VideoWidget::playVideo()
{
	QMediaPlaylist* playList = _mediaPlayer->playlist();
	playList->clear();
	playList->addMedia(QUrl::fromLocalFile(_currentRealLiveVideo.videoInformation().videoFilename()));
	_mediaPlayer->play();
}

void VideoWidget::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
	if (status == QMediaPlayer::BufferedMedia) {
		QMediaContent content = _mediaPlayer->media();
		QMediaResource resource = content.canonicalResource();

		qDebug() << "buffered" << resource.videoBitRate();
	}
}


void VideoWidget::realLiveVideoSelected(RealLiveVideo rlv)
{
	if (_playDelayTimer->isActive())
		_playDelayTimer->stop();
	_currentRealLiveVideo = rlv;

	_playDelayTimer->start();
}

void VideoWidget::courseSelected(int courseNr)
{
	if (courseNr == -1) {
		return;
	}

	if (!_currentRealLiveVideo.isValid())
		return;

	const Course& course = _currentRealLiveVideo.courses()[courseNr];
	float startDistance = course.start();
	quint32 frame = _currentRealLiveVideo.frameForDistance(startDistance);

	qDebug() << "duration = " << _mediaPlayer->duration();
	double totalFrames = (_mediaPlayer->duration() / 1000.0) * _currentRealLiveVideo.videoInformation().frameRate();
	double b = frame / totalFrames;
	_mediaPlayer->setPosition(_mediaPlayer->duration() * b);
}

