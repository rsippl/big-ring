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
            return QString("%1 km").arg(rlv.totalDistance());
        }
    }
    return QVariant();
}

QVariant RlvTableModel::headerData(int section, Qt::Orientation orientation, int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal) {
         switch (section) {
             case 0:
                 return tr("Name");

             case 1:
                 return tr("Length");

             default:
                 return QVariant();
         }
     }
     return QVariant();
 }
