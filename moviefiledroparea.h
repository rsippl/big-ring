#ifndef MOVIEFILEDROPAREA_H
#define MOVIEFILEDROPAREA_H

#include <QQuickItem>
#include <QUrl>

class MovieFileDropArea : public QQuickItem
{
	Q_OBJECT
public:
	explicit MovieFileDropArea(QQuickItem *parent = 0);
protected:
	void dragEnterEvent(QDragEnterEvent *);
signals:
	void movieDragged(QUrl url);
public slots:
	
};

#endif // MOVIEFILEDROPAREA_H
