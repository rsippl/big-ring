#ifndef VIDEOSCREENSHOTWIDGET_H
#define VIDEOSCREENSHOTWIDGET_H

#include <QWidget>

#include "reallifevideo.h"

class Thumbnailer;
namespace Ui {
class VideoScreenshotWidget;
}

/**
 * @brief subclass of QWidget that scales the screenshot pixmap, keeping the aspect ratio
 * of the image.
 */
class VideoScreenshotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoScreenshotWidget(QWidget *parent = 0);
    ~VideoScreenshotWidget();

public slots:
    void setVideo(RealLifeVideo& rlv);
    void setDistance(qreal distance);

protected:
    virtual void paintEvent(QPaintEvent *) override;
private:
    Thumbnailer* _thumbnailer;
    RealLifeVideo _rlv;
    qreal _distance;

    Ui::VideoScreenshotWidget *ui;
};

#endif // VIDEOSCREENSHOTWIDGET_H
