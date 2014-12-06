#include "videotileview.h"
#include "videotile.h"
#include <QtCore/QtDebug>
VideoTileView::VideoTileView(QWidget *parent) :
    QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene;
    setScene(scene);
    scene->setSceneRect(0, 0, 100, 100);
    setViewport(new QWidget);

}

void VideoTileView::resizeEvent(QResizeEvent *event)
{
    placeTiles();
    event->accept();
}

void VideoTileView::rlvsLoaded(RealLifeVideoList &rlvs)
{
    int i = 0;

    for (int j = 0; j < 100; j++) {
    for (const RealLifeVideo& rlv: rlvs) {
        qDebug() << "videos" << rlv.name();
        VideoTile* tile = new VideoTile(rlv);
        scene()->addItem(tile);
        _items.append(tile);
        i++;
    }
    }
    placeTiles();
}

void VideoTileView::placeTiles()
{
    for(int i = 0; i < _items.size(); ++i) {
        QGraphicsItem* tile = _items[i];

        int x = i % 2 == 0 ? 0 : rect().width() / 2;
        int y = i / 2 * tile->boundingRect().height();

        tile->setPos(mapToScene(x, y));

    }
}
