#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include "reallifevideo.h"

class ProfileWidget : public QWidget
{
	Q_OBJECT
public:
	explicit ProfileWidget(QWidget *parent = 0);
	void paintEvent(QPaintEvent *) override;
protected:

signals:
	
public slots:
	void rlvSelected(RealLifeVideo rlv);

private:
	void drawProfile(QPainter& p);
	RealLifeVideo _currentRlv;
};

#endif // PROFILEWIDGET_H
