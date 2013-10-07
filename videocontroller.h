#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QQueue>
#include <QTimer>

#include "cyclist.h"
#include "reallivevideo.h"
#include "videowidget.h"

class VideoController : public QObject
{
	Q_OBJECT
public:
	explicit VideoController(Cyclist& cyclist, VideoWidget* videoWidget, QObject *parent = 0);
	~VideoController();
	
	bool isBufferFull();

signals:
	void bufferFull(bool full);
	void playing(bool playing);
	void currentFrameRate(quint32 frameRate);

public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);
	void play(bool);

private slots:
	void videoLoaded();
	void playNextFrame();

	void framesReady(FrameList frames);
	void seekFinished(Frame frame);
private:
	struct FrameRequest {
		quint32 startFrame;
		quint32 nrOfFrames;
	};

	void displayFrame(quint32 frameToShow);
	void loadVideo(const QString& filename);
	void setPosition(quint32 frameNr);
	// reset buffers etc after choosing new video.
	void reset();
	Frame takeFrame();
	int determineFramesToSkip();
	void requestNewFrames(quint32 numberOfFrames);

	Cyclist& _cyclist;
	QLinkedList<Frame> _imageQueue;
	VideoDecoder* _videoDecoder;
	VideoWidget* const _videoWidget;
	QTimer _playTimer;
	bool _requestBusy;
	QDateTime _lastFrameRateSample;
	quint32 _currentFrameRate;
	quint32 _framesThisSecond;

	RealLiveVideo _currentRlv;
	quint32 _currentFrameNumber;
	quint32 _loadedFrameNumber;
};

#endif // VIDEOCONTROLLER_H
