#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTimer>

#include "cyclist.h"
#include "reallifevideo.h"
#include "videodecoder.h"

class VideoWidget;

class VideoController : public QObject, public VideoFrameSink
{
	Q_OBJECT
public:
	explicit VideoController(Cyclist& cyclist, VideoWidget* videoWidget, QObject *parent = 0);
	~VideoController();
	
	bool isBufferFull();

	virtual void offerFrame(Frame &frame) override;
	virtual void seekFinished(Frame &frame) override;
signals:
	void bufferFull(bool full);
	void playing(bool playing);
	void currentFrameRate(quint32 frameRate);

public slots:
	void realLiveVideoSelected(RealLifeVideo rlv);
	void courseSelected(int courseNr);
	void play(bool);

private slots:
	void playNextFrame();

private:
	void loadFrame(Frame& frame);
	void displayFrame(quint32 frameToShow);
	void setPosition(quint32 frameNr);
	// reset buffers etc after choosing new video.
	void reset();
	int determineFramesToSkip();
	void fillFrameBuffers();

	Cyclist& _cyclist;
	VideoDecoder* _videoDecoder;
	VideoWidget* const _videoWidget;
	QTimer _playTimer;
	QDateTime _lastFrameRateSample;
	quint32 _currentFrameRate;
	quint32 _framesThisSecond;

	RealLifeVideo _currentRlv;
	quint32 _currentFrameNumber;
	quint32 _loadedFrameNumber;
};

#endif // VIDEOCONTROLLER_H
