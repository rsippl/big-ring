#ifndef VIDEOTILEVIEW_H
#define VIDEOTILEVIEW_H

#include <QtCore/QList>
#include <QtWidgets/QGraphicsView>
#include "reallifevideo.h"
#include "videotile.h"

class VideoTile;
class VideoLightBox;

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
    virtual void scrollContentsBy(int dx, int dy);
    virtual void keyPressEvent(QKeyEvent *event);
private:
    void placeTiles();
    void showSelected(const RealLifeVideo& rlv);
    QList<VideoTile*> _items;
    VideoLightBox* _selectedVideoLightBox;
};

#endif // VIDEOTILEVIEW_H
