#ifndef VIDEOLISTVIEW_H
#define VIDEOLISTVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>

class VideoListModel;
class VideoDetailsWidget;

#include "reallifevideo.h"

class VideoListView : public QWidget
{
    Q_OBJECT

public:
    explicit VideoListView(QWidget *parent = 0);

signals:
    void videoSelected(RealLifeVideo& rlv);

public slots:
    void setVideos(RealLifeVideoList& rlvs);

private slots:
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected);

private:
    QListView* _listView;
    VideoDetailsWidget* _detailsWidget;
    VideoListModel* _videoListModel;
};

#endif // VIDEOLISTVIEW_H
