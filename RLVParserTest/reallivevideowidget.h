#ifndef REALLIVEVIDEOWIDGET_H
#define REALLIVEVIDEOWIDGET_H

#include <QWidget>

class QLabel;
class RealLiveVideo;

class RealLiveVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RealLiveVideoWidget(QWidget *parent = 0);

signals:

public slots:
	void newRealLiveVideo(RealLiveVideo& realLiveVideo);

private:
	QLabel* videoFilenameLabel;
	QLabel* frameRateLabel;
};

#endif // REALLIVEVIDEOWIDGET_H
