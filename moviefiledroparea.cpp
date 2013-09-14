#include "moviefiledroparea.h"
#include <QMimeData>
#include <QUrl>
#include <QtDebug>

MovieFileDropArea::MovieFileDropArea(QQuickItem *parent) :
	QQuickItem(parent)
{
	setFlag(QQuickItem::ItemAcceptsDrops);
}

void MovieFileDropArea::dragEnterEvent(QDragEnterEvent *event)
{
	qDebug() << "drag enter";
	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls()) {
		if (!mimeData->urls().empty()) {
			QUrl first = mimeData->urls()[0];
			qDebug() << "url = " << first;
			emit movieDragged(first);
			event->accept();
		}
	}
}
