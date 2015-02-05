#ifndef VIDEODETAILSWIDGET_H
#define VIDEODETAILSWIDGET_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>
#include "reallifevideo.h"

class ProfilePainter;
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
    QWidget* setupProfileLabel();

    void updateVideoScreenshotLabel(const RealLifeVideo& rlv, QPixmap& pixmap);

    RealLifeVideo _currentRlv;
    ProfilePainter* _profilePainter;
    Thumbnailer* _thumbnailer;
    QLabel* _nameLabel;
    QLabel* _distanceLabel;
    VideoScreenshotLabel* _videoScreenshotLabel;
    QLabel* _profileLabel;
};

#endif // VIDEODETAILSWIDGET_H
