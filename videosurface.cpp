#include "videosurface.h"
#include <QPainter>

VideoSurface::VideoSurface(QWidget *widget, QObject *parent) :
	QAbstractVideoSurface(parent), _widget(widget)
{
}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(
		QAbstractVideoBuffer::HandleType handleType) const
{
	if (handleType == QAbstractVideoBuffer::NoHandle) {
		return QList<QVideoFrame::PixelFormat>()
				<< QVideoFrame::Format_RGB32
				<< QVideoFrame::Format_ARGB32
				<< QVideoFrame::Format_ARGB32_Premultiplied
				<< QVideoFrame::Format_RGB565
				<< QVideoFrame::Format_RGB555;
	} else {
		return QList<QVideoFrame::PixelFormat>();
	}
}

bool VideoSurface::isFormatSupported(
		const QVideoSurfaceFormat &format, QVideoSurfaceFormat *similar) const
{
	Q_UNUSED(similar);

	const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
	const QSize size = format.frameSize();

	return imageFormat != QImage::Format_Invalid
			&& !size.isEmpty()
			&& format.handleType() == QAbstractVideoBuffer::NoHandle;
}

bool VideoSurface::start(const QVideoSurfaceFormat &format)
{
	const QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
	const QSize size = format.frameSize();

	if (imageFormat != QImage::Format_Invalid && !size.isEmpty()) {
		_imageFormat = imageFormat;
		_imageSize = size;
		_sourceRect = format.viewport();

		QAbstractVideoSurface::start(format);

		_widget->updateGeometry();
		updateVideoRect();

		return true;
	} else {
		return false;
	}
}

void VideoSurface::stop()
 {
	 _currentFrame = QVideoFrame();
	 _targetRect = QRect();

	 QAbstractVideoSurface::stop();

	 _widget->update();
 }

void VideoSurface::updateVideoRect()
 {
	 QSize size = surfaceFormat().sizeHint();
	 size.scale(_widget->size().boundedTo(size), Qt::KeepAspectRatio);

	 _targetRect = QRect(QPoint(0, 0), size);
	 _targetRect.moveCenter(_widget->rect().center());
}

QRect VideoSurface::videoRect() const
{
	return _targetRect;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
	if (surfaceFormat().pixelFormat() != frame.pixelFormat()
			|| surfaceFormat().frameSize() != frame.size()) {
		setError(IncorrectFormatError);
		stop();

		return false;
	} else {
		_currentFrame = frame;

		_widget->repaint(_targetRect);

		return true;
	}
}

void VideoSurface::paint(QPainter *painter)
{
	if (_currentFrame.map(QAbstractVideoBuffer::ReadOnly)) {
		const QTransform oldTransform = painter->transform();

		if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop) {
		   painter->scale(1, -1);
		   painter->translate(0, -_widget->height());
		}

		QImage image(
				_currentFrame.bits(),
				_currentFrame.width(),
				_currentFrame.height(),
				_currentFrame.bytesPerLine(),
				_imageFormat);

		painter->drawImage(_targetRect, image, _sourceRect);

		painter->setTransform(oldTransform);

		_currentFrame.unmap();
	}
}
