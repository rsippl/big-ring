#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QWidget>

class VideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
public:
	explicit VideoSurface(QWidget *widget, QObject *parent);
	QList<QVideoFrame::PixelFormat> supportedPixelFormats(
			QAbstractVideoBuffer::HandleType handleType) const;

	bool isFormatSupported(
			const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const;

	bool start(const QVideoSurfaceFormat &format);
	void stop();
	void updateVideoRect();
	QRect videoRect() const;
	bool present(const QVideoFrame &frame);
	void paint(QPainter* painter);

signals:
	
public slots:

private:
	QWidget* _widget;
	QVideoFrame _currentFrame;
	QImage::Format _imageFormat;
	QRect _targetRect;
	QSize _imageSize;
	QRect _sourceRect;
};

#endif // VIDEOSURFACE_H
