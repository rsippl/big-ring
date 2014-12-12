#ifndef VIDEOTILEVIEW_H
#define VIDEOTILEVIEW_H

#include <QtCore/QList>
#include <QtWidgets/QGraphicsView>
#include "reallifevideo.h"
#include "videotile.h"
class VideoTileView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VideoTileView(QWidget *parent = 0);

signals:

public slots:
    void rlvsLoaded(RealLifeVideoList& rlvs);
protected:
    void resizeEvent(QResizeEvent *);
private:
    void placeTiles();
    QList<VideoTile*> _items;
};

#endif // VIDEOTILEVIEW_H
