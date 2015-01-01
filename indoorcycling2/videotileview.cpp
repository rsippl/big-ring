#include "videotileview.h"

#include "videolightbox.h"
#include "videotile.h"
#include <QtCore/QtDebug>
#include <QtCore/QPropertyAnimation>
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
    if (event->key() == Qt::Key_Escape && _selectedVideoLightBox && _selectedVideoLightBox->isVisible()) {
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
            const QPointF position = mapToScene(0, y);
            tile->setGeometry(position.x(), position.y(), sceneRect().width(), _items[i]->boundingRect().height());

        }
    }
}

void VideoTileView::showSelected(const RealLifeVideo &rlv)
{
    _selectedVideoLightBox = new VideoLightBox(rlv);

    QPropertyAnimation* animation = new QPropertyAnimation(_selectedVideoLightBox, "geometry");
    animation->setDuration(1500);

    QPointF topLeft = mapToScene(viewport()->rect().topLeft());

    qDebug() << "viewport height = " << viewport()->height();

    QPointF lowerRight = mapToScene(viewport()->width(), viewport()->height());
    qDebug() << "topleft" << topLeft;
    qDebug() << "lowerright" << lowerRight;

    animation->setStartValue(QRectF(viewport()->rect().left(), viewport()->rect().center().y(), viewport()->rect().width(), 0));
    animation->setEndValue(QRectF(topLeft, lowerRight));
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    scene()->addItem(_selectedVideoLightBox);
    connect(_selectedVideoLightBox, &VideoLightBox::visibleChanged, _selectedVideoLightBox, [this]() {
        if (!_selectedVideoLightBox->isVisible()) {
            qDebug() << "removing and deleting light box";
            scene()->removeItem(this->_selectedVideoLightBox);
            this->_selectedVideoLightBox->deleteLater();
            this->_selectedVideoLightBox = nullptr;
        }
    });
}


