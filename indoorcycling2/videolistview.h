#ifndef VIDEOLISTVIEW_H
#define VIDEOLISTVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>

class VideoListModel;

#include "reallifevideo.h"

class VideoListView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoListView(QWidget *parent = 0);

signals:

public slots:
    void setVideos(RealLifeVideoList& rlvs);

private:
    QListView* _listView;
    VideoListModel* _videoListModel;
};

#endif // VIDEOLISTVIEW_H
