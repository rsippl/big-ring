#ifndef VIDEODETAILSWIDGET_H
#define VIDEODETAILSWIDGET_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include "reallifevideo.h"

class Thumbnailer;
class VideoScreenshotLabel;

class VideoDetailsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoDetailsWidget(QWidget *parent = 0);

signals:
    void playClicked(RealLifeVideo& rlv);
public slots:
    void setVideo(RealLifeVideo& rlv);

private:
    QWidget* setupDetails();
    QWidget* setupVideoScreenshot();

    void updateVideoScreenshotLabel(const RealLifeVideo& rlv, QPixmap& pixmap);

    RealLifeVideo _currentRlv;
    Thumbnailer* _thumbnailer;
    QLabel* _nameLabel;
    QLabel* _distanceLabel;
    VideoScreenshotLabel* _videoScreenshotLabel;
};

#endif // VIDEODETAILSWIDGET_H
