#ifndef VIDEOTILE_H
#define VIDEOTILE_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"
class VideoTile : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit VideoTile(const RealLifeVideo rlv, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
signals:

public slots:
private:
    const RealLifeVideo _rlv;
};

#endif // VIDEOTILE_H
