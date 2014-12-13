#include "videotileview.h"
#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtOpenGL/QGLWidget>

VideoTileView::VideoTileView(QWidget *parent) :
    QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene;
    setScene(scene);
    scene->setSceneRect(0, 0, 120, 1000);
    setViewport(new QGLWidget);
//    viewport()->setShortcutAutoRepeat();

}

void VideoTileView::resizeEvent(QResizeEvent *event)
{
    scene()->setSceneRect(0, 0, event->size().width(), _items.size() * 100);
    placeTiles();
    event->accept();
}

void VideoTileView::rlvsLoaded(RealLifeVideoList &rlvs)
{
    for (const RealLifeVideo& rlv: rlvs) {
        qDebug() << "videos" << rlv.name();
        VideoTile* tile = new VideoTile(rlv);
        scene()->addItem(tile);
        _items.append(tile);
    }

    placeTiles();

}

void VideoTileView::placeTiles()
{
    if (!_items.isEmpty()) {
        scene()->setSceneRect(0, 0, viewport()->width(), _items.size() * _items[0]->boundingRect().height());
        for(int i = 0; i < _items.size(); ++i) {
            VideoTile* tile = _items[i];
            int y = i * _items[i]->boundingRect().height();
            tile->setWidth(rect().width());
            tile->setPos(0, y);
        }
    }
}


