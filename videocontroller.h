#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QQueue>
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
	
	bool isBufferFull();

signals:
	void distanceChanged(float distance);
	void slopeChanged(float slope);
	void altitudeChanged(float altitude);
	void bufferFull(bool full);
	void playing(bool playing);

public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);
	void play(bool);


private slots:
	void videoLoaded();
	void updateVideo();
	void displayFrame();
	void framesReady(FrameList frames, quint32 requestId);
	void seekFinished();
private:
	void updateDistance();
	void loadVideo(const QString& filename);
	void setDistance(float distance);
	void setPosition(quint32 frameNr);
	// reset buffers etc after choosing new video.
	void reset();
	Frame takeFrame();
	void requestNewFrames(quint32 numberOfFrames);

	QQueue<Frame> _imageQueue;
	VideoDecoder _videoDecoder;
	QThread _decoderThread;
	VideoWidget* const _videoWidget;
	QTimer _updateTimer;
	QTimer _playTimer;

	RealLiveVideo _currentRlv;
	quint32 _currentFrameNumber;
	float _currentDistance;
	bool _running;
	bool _newFramesRequested;
	quint32 _frameRequestId;

	qint64 _lastTime;

};

#endif // VIDEOCONTROLLER_H
