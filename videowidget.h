#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QVideoWidget>
#include <QMediaPlayer>

#include "reallivevideo.h"
#include "videodecoder.h"

class QThread;

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
signals:
	void videoDurationAvailable(qint64 durationMs);

private:

	QVideoWidget *_videoWidget;
	QMediaPlayer *_mediaPlayer;

	RealLiveVideo _currentRealLiveVideo;
};

#endif // VIDEOWIDGET_H
