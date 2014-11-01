#include "rlvtablemodel.h"

RlvTableModel::RlvTableModel(RealLifeVideoList& rlvList, QObject *parent) :
    QAbstractTableModel(parent), _rlvList(rlvList)
{
}

int RlvTableModel::columnCount(const QModelIndex&) const
{
    return 2;
}

int RlvTableModel::rowCount(const QModelIndex&) const
{
    return _rlvList.size();
}

QVariant RlvTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        const RealLifeVideo& rlv = _rlvList[index.row()];
        switch(index.column()) {
        case 0:
            return rlv.name();
        case 1:
            return QVariant::fromValue(rlv.totalDistance());
        }
    }
    return QVariant();
}
