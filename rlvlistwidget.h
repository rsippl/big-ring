#ifndef RLVLISTWIDGET_H
#define RLVLISTWIDGET_H

#include <QWidget>
#include "reallifevideo.h"
namespace Ui {
    class RlvListWidget;
}

class RlvListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RlvListWidget(QWidget *parent = 0);
    ~RlvListWidget();

public slots:
    void setRealLiveVideos(RealLifeVideoList rlvs);

signals:
    void realLiveVideoSelected(RealLifeVideo realLiveVideo);

private slots:
    void selectionChanged(int row);
private:
    Ui::RlvListWidget *ui;
    RealLifeVideoList realLiveVideoList;
};

#endif // RLVLISTWIDGET_H
