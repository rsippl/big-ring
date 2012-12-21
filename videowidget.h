#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QGLWidget>

#include "reallivevideo.h"
#include "videodecoder.h"

class QThread;

class VideoWidget : public QGLWidget
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget *parent = 0);
	virtual ~VideoWidget();

	void paintGL();
	void resizeGL(int w, int h);

	void loadVideo(const QString& filename);
	void playVideo();
	void stop();
	void setRate(float framesPerSecond);
	void setPosition(quint32 frameNr);

protected:
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private slots:
	void frameTimeout();
	void videoLoaded();

private:
	ImageQueue _imageQueue;
	VideoDecoder* _videoDecoder;
	QTimer* _playTimer;
	QThread* _decoderThread;
	QImage _currentImage;
	GLuint _texture;
};

#endif // VIDEOWIDGET_H
