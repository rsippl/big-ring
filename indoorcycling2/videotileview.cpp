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
    setViewport(new QGLWidget);

    _mainWidget = new QGraphicsWidget(nullptr, Qt::Widget);

    _layout = new QGraphicsGridLayout();
    _layout->setSpacing(12.0);

    _mainWidget->setLayout(_layout);

    _layout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _mainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _mainWidget->setPos(mapToScene(0, 0));
    scene->addItem(_mainWidget);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setScene(scene);
    _mainWidget->setGeometry(scene->sceneRect());

    setMinimumSize(_mainWidget->minimumSize().toSize());
}

void VideoTileView::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    _mainWidget->setGeometry(0, 0, event->size().width(), event->size().height());
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
    if (_selectedVideoLightBox) {
        _selectedVideoLightBox->setPos(mapToScene(10, 10));
    }
}

void VideoTileView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && _selectedVideoLightBox && _selectedVideoLightBox->isVisible()) {
        scene()->removeItem(_selectedVideoLightBox.data());
        _selectedVideoLightBox.reset();
    }
}

void VideoTileView::rlvsLoaded(RealLifeVideoList &rlvs)
{
    for (RealLifeVideo& rlv: rlvs) {
        VideoTile* tile = new VideoTile(rlv, _mainWidget);

        connect(tile, &VideoTile::selected, tile, [this](RealLifeVideo& rlv) {
             this->showSelected(rlv);
        });
        _items.append(tile);
    }

    placeTiles();

}

void VideoTileView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QBrush brush(Qt::gray);
    painter->setBrush(brush);
    painter->drawRect(rect);
}

void VideoTileView::placeTiles()
{
    if (!_items.isEmpty()) {
        for(int i = 0; i < _items.size(); ++i) {
            VideoTile* tile = _items[i];
            tile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            qDebug() << "adding item at" << i%2 << i /2;
            _layout->addItem(tile, i / 2, i % 2);
        }
    }

    setMinimumWidth(_mainWidget->minimumWidth() + 20);
}

void VideoTileView::showSelected(RealLifeVideo &rlv)
{
    _selectedRlv = rlv;
    _selectedVideoLightBox.reset(new VideoLightBox(rlv));
    _selectedVideoLightBox->setZValue(1.0);

    QPropertyAnimation* animation = new QPropertyAnimation(_selectedVideoLightBox.data(), "geometry");
    animation->setDuration(350);

    QPointF topLeft = mapToScene(viewport()->rect().topLeft());

    QPointF lowerRight = mapToScene(viewport()->width(), viewport()->height());

    animation->setStartValue(QRectF(viewport()->rect().left(), viewport()->rect().center().y(),
                                    viewport()->rect().width(), 0));
    animation->setEndValue(QRectF(topLeft, lowerRight));
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    scene()->addItem(_selectedVideoLightBox.data());
    connect(_selectedVideoLightBox.data(), &VideoLightBox::visibleChanged, _selectedVideoLightBox.data(), [this]() {
        this->removeSelectedVideoLightBox();
    });
    connect(_selectedVideoLightBox.data(), &VideoLightBox::playRequested, _selectedVideoLightBox.data(), [=]() {
        qDebug() << "rlv selected for playing:" << rlv.name();
        emit startRlv(_selectedRlv);
    });

}

void VideoTileView::removeSelectedVideoLightBox()
{
    if (!_selectedVideoLightBox->isVisible()) {
        scene()->removeItem(this->_selectedVideoLightBox.data());
    }
}


