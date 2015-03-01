#include "videoscreenshotwidget.h"
#include "ui_videoscreenshotwidget.h"

#include <QtGui/QPainter>

VideoScreenshotWidget::VideoScreenshotWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoScreenshotWidget)
{
    ui->setupUi(this);
}

VideoScreenshotWidget::~VideoScreenshotWidget()
{
    delete ui;
}


void VideoScreenshotWidget::setPixmap(const QPixmap &pixmap)
{
    _videoScreenshot = pixmap;
    repaint();
}

void VideoScreenshotWidget::paintEvent(QPaintEvent *)
{
    QPixmap scaled = _videoScreenshot.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QRect scaledRect = scaled.rect();
    scaledRect.moveCenter(rect().center());

    QPainter painter(this);
    painter.fillRect(rect(), palette().background());
    painter.drawPixmap(scaledRect, scaled);
}

