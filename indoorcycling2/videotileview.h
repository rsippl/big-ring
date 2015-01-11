#ifndef VIDEOTILEVIEW_H
#define VIDEOTILEVIEW_H

#include <QtCore/QList>
#include <QtWidgets/QGraphicsGridLayout>
#include <QtWidgets/QGraphicsView>
#include <QtCore/QScopedPointer>
#include <QtCore/QScopedPointerDeleteLater>

#include "reallifevideo.h"
#include "videotile.h"
#include "videolightbox.h"

class VideoTileView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit VideoTileView(QWidget *parent = 0);

signals:

public slots:
    void rlvsLoaded(RealLifeVideoList& rlvs);

signals:
    void startRlv(RealLifeVideo& rlv);

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *) override;
    virtual void scrollContentsBy(int dx, int dy) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
private:
    void placeTiles();
    void showSelected(RealLifeVideo &rlv);
    void removeSelectedVideoLightBox();

    QGraphicsWidget* _mainWidget;
    QGraphicsGridLayout* _layout;
    QList<VideoTile*> _items;
    QScopedPointer<VideoLightBox,QScopedPointerObjectDeleteLater<VideoLightBox>> _selectedVideoLightBox;
    RealLifeVideo _selectedRlv;
};

#endif // VIDEOTILEVIEW_H
