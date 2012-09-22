#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H


#include <QTimer>
#include <QVideoWidget>
#include <QMediaPlayer>

#include "reallivevideo.h"
#include "videodecoder.h"

class QLabel;


class VideoWidget : public QWidget
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget *parent = 0);
	virtual ~VideoWidget();

	void loadVideo(const QString& videoFilename);
	void playVideo();

	qint64 videoDuration() const;

	void setPosition(qint64 position);
	void setRate(float rate);

	void setSpeed(float speed);
	void setDistance(float distance);
signals:
	void videoDurationAvailable(qint64 durationMs);

private:

	QVideoWidget *_videoWidget;
	QMediaPlayer *_mediaPlayer;

	QLabel *_speedLabel;
	QLabel *_distanceLabel;

	RealLiveVideo _currentRealLiveVideo;
};

#endif // VIDEOWIDGET_H
