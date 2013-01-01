#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QPair>
#include <QQueue>
#include <QThread>
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

public slots:
	void realLiveVideoSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);
	void play(bool);


private slots:
	void videoLoaded();
	void displayFrame();
	void framesReady(FrameList frames, quint32 requestId);
	void seekFinished();
private:
	void loadVideo(const QString& filename);
	void setPosition(quint32 frameNr);
	// reset buffers etc after choosing new video.
	void reset();
	Frame takeFrame();
	void requestNewFrames(quint32 numberOfFrames);

	Cyclist& _cyclist;
	QQueue<Frame> _imageQueue;
	VideoDecoder _videoDecoder;
	QThread _decoderThread;
	VideoWidget* const _videoWidget;
	QTimer _playTimer;

	RealLiveVideo _currentRlv;
	quint32 _currentFrameNumber;
	bool _running;
	bool _newFramesRequested;
	quint32 _frameRequestId;
};

#endif // VIDEOCONTROLLER_H
