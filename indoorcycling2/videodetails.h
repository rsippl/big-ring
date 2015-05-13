#ifndef VIDEODETAILS_H
#define VIDEODETAILS_H

#include <QWidget>

#include "antcentraldispatch.h"
#include "reallifevideo.h"

class QuantityPrinter;

namespace Ui {
class VideoDetails;
}

class VideoDetails : public QWidget
{
    Q_OBJECT

public:
    explicit VideoDetails(indoorcycling::AntCentralDispatch* antCentralDispatch, QWidget *parent = 0);
    ~VideoDetails();

public slots:
    void setVideo(RealLifeVideo& rlv);
signals:
    void playClicked(RealLifeVideo& rlv, int courseNr);
private slots:
    void on_startButton_clicked();

    void on_courseListWidget_currentRowChanged(int currentRow);

    void on_newCourseButton_clicked();

private:
    RealLifeVideo _currentRlv;
    int _courseIndex;
    QuantityPrinter* const _quantityPrinter;
    Ui::VideoDetails *ui;
    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
};

#endif // VIDEODETAILS_H
