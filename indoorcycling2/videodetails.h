#ifndef VIDEODETAILS_H
#define VIDEODETAILS_H

#include <QWidget>

#include "reallifevideo.h"

class Thumbnailer;
class AltitudeProfileWidget;
class QuantityPrinter;
class VideoScreenshotWidget;

namespace Ui {
class VideoDetails;
}

class VideoDetails : public QWidget
{
    Q_OBJECT

public:
    explicit VideoDetails(QWidget *parent = 0);
    ~VideoDetails();

public slots:
    void setVideo(RealLifeVideo& rlv);
signals:
    void playClicked(RealLifeVideo& rlv, int courseNr);
private slots:
    void on_pushButton_clicked();

    void on_courseListWidget_currentRowChanged(int currentRow);

private:
    void updateVideoScreenshotLabel(const RealLifeVideo& rlv, const qreal distance, QPixmap& pixmap);

    RealLifeVideo _currentRlv;
    int _courseIndex;
    Thumbnailer* _thumbnailer;
//    VideoScreenshotWidget* _videoScreenshotWidget;
//    AltitudeProfileWidget* _profileLabel;
    QuantityPrinter* _quantityPrinter;
    Ui::VideoDetails *ui;
};

#endif // VIDEODETAILS_H
