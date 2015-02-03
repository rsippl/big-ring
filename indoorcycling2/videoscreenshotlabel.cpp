#include "videoscreenshotlabel.h"

#include <QtCore/QtDebug>
#include <QtGui/QPainter>

VideoScreenshotLabel::VideoScreenshotLabel(QWidget *parent) :
    QLabel(parent)
{
    // empty
}


void VideoScreenshotLabel::setPixmap(const QPixmap &pixmap)
{
    _videoScreenshot = pixmap;
    scaleAndPlacePixmap();
}

void VideoScreenshotLabel::resizeEvent(QResizeEvent *resizeEvent)
{
    scaleAndPlacePixmap();
    QLabel::resizeEvent(resizeEvent);
}

void VideoScreenshotLabel::scaleAndPlacePixmap()
{
    QPixmap scaled = _videoScreenshot.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QRect scaledRect = scaled.rect();
    scaledRect.moveCenter(rect().center());

    // paint scaled pixmap in the center of a new target pixmap
    QPixmap target(size());
    QPainter painter(&target);


    painter.fillRect(target.rect(), palette().background());
    painter.drawPixmap(scaledRect, scaled);

    QLabel::setPixmap(target);
}
