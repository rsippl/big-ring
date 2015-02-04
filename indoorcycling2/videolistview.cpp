#include "videolistview.h"

#include <QtCore/QtDebug>
#include <QtCore/QItemSelection>
#include <QtWidgets/QHBoxLayout>


#include "videolistmodel.h"
#include "videoitemdelegate.h"
#include "videodetailswidget.h"

VideoListView::VideoListView(QWidget *parent) :
    QWidget(parent), _listView(new QListView(this)), _detailsWidget(new VideoDetailsWidget(this)), _videoListModel(new VideoListModel)
{
    QHBoxLayout* layout = new QHBoxLayout;
    _listView->setModel(_videoListModel);
    _listView->setItemDelegate(new VideoItemDelegate(this));
    _listView->setMaximumWidth(500);
    connect(_listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &VideoListView::selectionChanged);

    layout->addWidget(_listView);
    layout->addWidget(_detailsWidget);

    connect(_detailsWidget, &VideoDetailsWidget::playClicked, this, &VideoListView::videoSelected);


    setLayout(layout);
}

void VideoListView::setVideos(RealLifeVideoList &rlvs)
{
    _videoListModel->setVideos(rlvs);
    QModelIndex index = _videoListModel->index(0);
    if (index.isValid()) {
        _listView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    }
}

void VideoListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "selection changed" << _videoListModel->data(selected.indexes()[0], Qt::DisplayRole);
    RealLifeVideo rlv = selected.indexes()[0].data(VideoDataRole).value<RealLifeVideo>();
    _detailsWidget->setVideo(rlv);
}
