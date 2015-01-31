#ifndef VIDEOLISTMODEL_H
#define VIDEOLISTMODEL_H

#include <QtCore/QAbstractListModel>
#include "reallifevideo.h"

class VideoListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit VideoListModel(QObject *parent = 0);


    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
signals:

public slots:
    void setVideos(RealLifeVideoList& rlvs);

private:
    RealLifeVideoList _rlvs;
};

#endif // VIDEOLISTMODEL_H
