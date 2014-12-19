#include "videotileview.h"

#include "videolightbox.h"
#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtOpenGL/QGLWidget>

VideoTileView::VideoTileView(QWidget *parent) :
    QGraphicsView(parent), _selectedVideoLightBox(nullptr)
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
    if (_selectedVideoLightBox) {
        QPointF lowerRight = mapToScene(viewport()->rect().bottomRight());
        QPointF topLeft = mapToScene(viewport()->rect().topLeft());
        _selectedVideoLightBox->setGeometry(QRectF(0, 0, lowerRight.x() - topLeft.x() - 20, lowerRight.y() - topLeft.y() - 20));
        _selectedVideoLightBox->setPos(mapToScene(10, 10));
    }
    event->accept();
}

void VideoTileView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    qDebug() << "scroll by" << dx << dy;
    if (_selectedVideoLightBox) {
        _selectedVideoLightBox->setPos(mapToScene(10, 10));
    }
}

void VideoTileView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && _selectedVideoLightBox->isVisible()) {
        scene()->removeItem(_selectedVideoLightBox);
        _selectedVideoLightBox->deleteLater();
        _selectedVideoLightBox = nullptr;
    }
}

void VideoTileView::rlvsLoaded(RealLifeVideoList &rlvs)
{
    for (const RealLifeVideo& rlv: rlvs) {
        qDebug() << "videos" << rlv.name();
        VideoTile* tile = new VideoTile(rlv);
        connect(tile, &VideoTile::selected, this, [this](const RealLifeVideo& rlv) {
            this->showSelected(rlv);
        });

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
            tile->setGeometry(0, 0, sceneRect().width(), _items[i]->boundingRect().height());
            tile->setPos(0, y);
        }
    }
}

void VideoTileView::showSelected(const RealLifeVideo &rlv)
{
    QPointF lowerRight = mapToScene(viewport()->rect().bottomRight());
    QPointF topLeft = mapToScene(viewport()->rect().topLeft());
    _selectedVideoLightBox = new VideoLightBox(rlv, QSizeF(lowerRight.x() - topLeft.x() - 20, lowerRight.y() - topLeft.y() - 20));
    _selectedVideoLightBox->setGeometry(QRectF(0, 0, lowerRight.x() - topLeft.x() - 20, lowerRight.y() - topLeft.y() - 20));
    _selectedVideoLightBox->setPos(mapToScene(10, 10));
    scene()->addItem(_selectedVideoLightBox);
}


