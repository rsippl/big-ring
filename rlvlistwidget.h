#ifndef RLVLISTWIDGET_H
#define RLVLISTWIDGET_H

#include <QWidget>
#include "reallivevideo.h"
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
    void setRealLiveVideos(RealLiveVideoList rlvs);

signals:
    void realLiveVideoSelected(RealLiveVideo realLiveVideo);

private slots:
    void selectionChanged(int row);
private:
    Ui::RlvListWidget *ui;
    RealLiveVideoList realLiveVideoList;
};

#endif // RLVLISTWIDGET_H
