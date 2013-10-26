#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include "reallifevideo.h"

/**
 * @brief Widget that shows an altitude profile and the current position of the cyclist.
 */
class ProfileWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ProfileWidget(QWidget *parent = 0);

protected:
	void paintEvent(QPaintEvent *) override;
	void resizeEvent(QResizeEvent *) override;
signals:
	
public slots:
	void rlvSelected(RealLifeVideo rlv);

	void distanceChanged(float distance);

private:
	void drawProfile();

	qreal distanceToX(float distance) const;

	RealLifeVideo _currentRlv;
	QPixmap _profilePixmap;
	float _currentDistance;
};

#endif // PROFILEWIDGET_H
