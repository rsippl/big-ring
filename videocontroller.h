#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>

#include "reallivevideo.h"
#include "videowidget.h"

class VideoController : public QObject
{
	Q_OBJECT
public:
	explicit VideoController(VideoWidget* videoWidget, QObject *parent = 0);
	
signals:
	
public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);

private slots:
	void playVideo();
	void updateVideo();
	void videoDurationAvailable(qint64 durationMs);

private:
	VideoWidget* const _videoWidget;
	QTimer* const _playDelayTimer;
	QTimer* const _updateTimer;

	RealLiveVideo _currentRlv;
	float _currentDistance;
};

#endif // VIDEOCONTROLLER_H
