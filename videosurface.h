#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

#include <QWidget>
#include <QtMultimediaKit/QAbstractVideoSurface>

class VideoSurface : public QAbstractVideoSurface
{
	Q_OBJECT
public:
	explicit VideoSurface(QWidget *parent, QObject *parent = 0);
	
signals:
	
public slots:
	
};

#endif // VIDEOSURFACE_H
