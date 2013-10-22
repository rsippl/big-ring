#include "profilewidget.h"

#include <QPair>
#include <QPainter>
#include <QtDebug>

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
}

void ProfileWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QBrush brush(Qt::black);
	p.setBackground(brush);

	p.fillRect(rect(), brush);

	if (_currentRlv.isValid()) {
		drawProfile(p);
	}
}

void ProfileWidget::drawProfile(QPainter& p)
{
	auto profileEntries = _currentRlv.profile().entries();
	auto minAndMaxAltitude = findMinimumAndMaximumAltiude(profileEntries);
	float minimumAltitude = minAndMaxAltitude.first;
	float maximumAltitude = minAndMaxAltitude.second;

	float altitudeDiff = maximumAltitude - minimumAltitude;

	float totalDistance = _currentRlv.totalDistance();

	QPainterPath path;
	path.moveTo(0, 0);
	foreach(const ProfileEntry& entry, profileEntries) {
		float x = (entry.totalDistance() / totalDistance) * width();
		float y = height() - (((entry.altitude() - minimumAltitude) / altitudeDiff) * height());
		path.lineTo(x, y);
	}
	path.lineTo(width(), 0);
	path.lineTo(0, 0);

	QBrush brush(Qt::white);
	p.fillPath(path, brush);
}

void ProfileWidget::rlvSelected(RealLifeVideo rlv)
{
	_currentRlv = rlv;
	repaint();
}
