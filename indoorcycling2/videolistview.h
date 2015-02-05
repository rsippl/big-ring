#ifndef VIDEOLISTVIEW_H
#define VIDEOLISTVIEW_H

#include <QtCore/QSortFilterProxyModel>
#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QLineEdit>

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
    void listViewContentChanged();

    QLineEdit* _filterLineEdit;
    QListView* _listView;
    QSortFilterProxyModel* _filterProxyModel;
    VideoDetailsWidget* _detailsWidget;
    VideoListModel* _videoListModel;

};

#endif // VIDEOLISTVIEW_H
