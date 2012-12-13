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
	void distanceChanged(float distance);
	void slopeChanged(float slope);

public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);

private slots:
	void updateVideo();
private:
	void setDistance(float distance);

	VideoWidget* const _videoWidget;
	QTimer* const _updateTimer;

	RealLiveVideo _currentRlv;

	qint64 _lastTime;
	float _currentDistance;
	bool _running;
};

#endif // VIDEOCONTROLLER_H
