#include "profilewidget.h"

#include <QPair>
#include <QPainter>
#include <QtDebug>
#include <QTimer>

namespace
{
QPair<float,float> findMinimumAndMaximumAltiude(const QList<ProfileEntry>& profileEntries)
{
	float minimumAltitude = std::numeric_limits<float>::max();
	float maximumAltitude = std::numeric_limits<float>::min();
	foreach (const ProfileEntry& entry, profileEntries) {
		minimumAltitude = qMin(minimumAltitude, entry.altitude());
		maximumAltitude = qMax(maximumAltitude, entry.altitude());
	}
	return qMakePair(minimumAltitude, maximumAltitude);
}
}
ProfileWidget::ProfileWidget(QWidget *parent) :
	QWidget(parent)
{
	QTimer* repaintTimer = new QTimer(this);
	repaintTimer->setInterval(1000);
	connect(repaintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
	repaintTimer->start();
}

void ProfileWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QBrush brush(Qt::black);
	p.setBackground(brush);

	p.fillRect(rect(), brush);

	if (_currentRlv.isValid()) {
		QBrush brush(Qt::white);
		p.fillPath(_profilePath, brush);
	}

	if (_currentDistance > 0) {
		QPen pen(Qt::SolidLine);
		pen.setColor(QColor(Qt::blue));
		pen.setWidth(5);
		p.setPen(pen);

		int x = static_cast<int>(distanceToX(_currentDistance));
		p.drawLine(x, 0, x, height());
	}
}

void ProfileWidget::resizeEvent(QResizeEvent *)
{
	_profilePath = drawProfile();
	repaint();
}

QPainterPath ProfileWidget::drawProfile()
{
	auto profileEntries = _currentRlv.profile().entries();
	auto minAndMaxAltitude = findMinimumAndMaximumAltiude(profileEntries);
	float minimumAltitude = minAndMaxAltitude.first;
	float maximumAltitude = minAndMaxAltitude.second;

	float altitudeDiff = maximumAltitude - minimumAltitude;

	QPainterPath path;
	path.moveTo(0, height());
	foreach(const ProfileEntry& entry, profileEntries) {
		qreal x = distanceToX(entry.totalDistance());
		float y = height() - (((entry.altitude() - minimumAltitude) / altitudeDiff) * height());
		path.lineTo(x, y);
	}
	path.lineTo(width(), height());
	path.lineTo(0, height());

	return path;


}

qreal ProfileWidget::distanceToX(float distance) const
{
	return (distance / _currentRlv.totalDistance()) * width();
}

void ProfileWidget::rlvSelected(RealLifeVideo rlv)
{
	_currentRlv = rlv;
	_profilePath = drawProfile();
	repaint();
}

void ProfileWidget::distanceChanged(float distance)
{
	qDebug() << "distance =" << distance;
	_currentDistance = distance;
}
