#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "reallivevideo.h"
#include "videowidget.h"

class VideoController : public QObject
{
	Q_OBJECT
public:
	explicit VideoController(VideoWidget* videoWidget, QObject *parent = 0);
	~VideoController();
	
signals:
	void distanceChanged(float distance);
	void slopeChanged(float slope);
	void altitudeChanged(float altitude);

public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);
	void play(bool);

private slots:
	void updateVideo();
	void displayFrame();
private:
	void loadVideo(const QString& filename);
	void setDistance(float distance);
	void setPosition(quint32 frameNr);


	ImageQueue _imageQueue;
	VideoDecoder _videoDecoder;
	QThread _decoderThread;
	VideoWidget* const _videoWidget;
	QTimer _updateTimer;
	QTimer _playTimer;

	RealLiveVideo _currentRlv;

	qint64 _lastTime;
	float _currentDistance;
	bool _running;
};

#endif // VIDEOCONTROLLER_H
