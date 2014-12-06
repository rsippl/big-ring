#ifndef VIDEOTILEVIEW_H
#define VIDEOTILEVIEW_H

#include <QtCore/QList>
#include <QtWidgets/QGraphicsView>
#include "reallifevideo.h"
class VideoTileView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VideoTileView(QWidget *parent = 0);
    void resizeEvent(QResizeEvent *);
signals:

public slots:
    void rlvsLoaded(RealLifeVideoList& rlvs);
private:
    void placeTiles();
    QList<QGraphicsItem*> _items;
};

#endif // VIDEOTILEVIEW_H
