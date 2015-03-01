#include "videoscreenshotwidget.h"
#include "ui_videoscreenshotwidget.h"

#include <QtGui/QPainter>

#include "thumbnailer.h"
VideoScreenshotWidget::VideoScreenshotWidget(QWidget *parent) :
    QWidget(parent), _thumbnailer(new Thumbnailer(this)), _distance(0),
    ui(new Ui::VideoScreenshotWidget)
{
    ui->setupUi(this);

    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, this, [=](const RealLifeVideo& rlv, const qreal distance, QPixmap) {
        if (rlv == _rlv && qFuzzyCompare(distance, _distance)) {
            update();
        }
    });
}

VideoScreenshotWidget::~VideoScreenshotWidget()
{
    delete ui;
}


void VideoScreenshotWidget::setVideo(RealLifeVideo& rlv)
{
    _rlv = rlv;
    _distance = 0;
    update();
}

void VideoScreenshotWidget::setDistance(qreal distance)
{
    _distance = distance;
    update();
}

void VideoScreenshotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), palette().background());

    QPixmap videoScreenshot = _thumbnailer->thumbnailFor(_rlv, _distance);
    if (!videoScreenshot.isNull()) {
        QPixmap scaled = videoScreenshot.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QRect scaledRect = scaled.rect();
        scaledRect.moveCenter(rect().center());

        painter.drawPixmap(scaledRect, scaled);
    }
}

