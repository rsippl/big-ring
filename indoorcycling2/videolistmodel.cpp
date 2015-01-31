#include "videolistmodel.h"

#include <QtCore/QtDebug>

VideoListModel::VideoListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

void VideoListModel::setVideos(RealLifeVideoList &rlvs)
{
    qDebug() << "settings videos in model";
    beginInsertRows(QModelIndex(), 0, 0);
    _rlvs = rlvs;
    endInsertRows();
}

int VideoListModel::rowCount(const QModelIndex &parent) const
{
    return _rlvs.size();
}

QVariant VideoListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() <= _rlvs.size()) {
        switch(role) {
        case Qt::DisplayRole:
            return QVariant::fromValue(_rlvs[index.row()].name());

        case Qt::ToolTipRole:
            return QVariant::fromValue(_rlvs[index.row()].totalDistance());
        }
    }
    return QVariant();
}
