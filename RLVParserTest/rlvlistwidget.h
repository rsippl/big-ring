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

private:
    Ui::RlvListWidget *ui;
};

#endif // RLVLISTWIDGET_H
