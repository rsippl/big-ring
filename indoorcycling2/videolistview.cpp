#include "videolistview.h"

#include <QtWidgets/QVBoxLayout>

#include "videolistmodel.h"
#include "videoitemdelegate.h"

VideoListView::VideoListView(QWidget *parent) :
    QWidget(parent), _listView(new QListView), _videoListModel(new VideoListModel)
{
    QVBoxLayout* layout = new QVBoxLayout;
    _listView->setModel(_videoListModel);
    _listView->setItemDelegate(new VideoItemDelegate(this));
    layout->addWidget(_listView);

    setLayout(layout);
}

void VideoListView::setVideos(RealLifeVideoList &rlvs)
{
    _videoListModel->setVideos(rlvs);
}
