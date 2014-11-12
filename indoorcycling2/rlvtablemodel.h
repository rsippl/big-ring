#ifndef RLVTABLEMODEL_H
#define RLVTABLEMODEL_H

#include <QObject>
#include <QtCore/QAbstractTableModel>

#include "reallifevideo.h"

class RlvTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RlvTableModel(RealLifeVideoList& rlvList, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
signals:

public slots:

private:
    RealLifeVideoList _rlvList;
};

#endif // RLVTABLEMODEL_H
