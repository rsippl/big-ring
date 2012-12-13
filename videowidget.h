#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QGLWidget>

#include "reallivevideo.h"
#include "videodecoder.h"

class QThread;

class VideoWidget : public QGLWidget, public VideoImageHandler
{
	Q_OBJECT
public:
	explicit VideoWidget(QWidget *parent = 0);
	virtual ~VideoWidget();

	void paintGL();
	void resizeGL(int w, int h);
	virtual void handleImage(const QImage& image);

	void loadVideo(const QString& filename);
	void playVideo();
	void setRate(float framesPerSecond);
	void setPosition(quint32 frameNr, float frameRate);
signals:

protected:
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
private slots:
	void frameReady(quint32 frameNr);

private:
	VideoDecoder* _videoDecoder;

	QTimer* _playTimer;
	QThread* _decoderThread;
	QImage _glImage;
	GLuint _texture;
};

#endif // VIDEOWIDGET_H
