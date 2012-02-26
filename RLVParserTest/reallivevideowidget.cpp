#include "reallivevideowidget.h"

#include "reallivevideo.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QGroupBox>

#include <QtDebug>

RealLiveVideoWidget::RealLiveVideoWidget(QWidget *parent) :
	QWidget(parent),
	videoFilenameLabel(new QLabel),
	frameRateLabel(new QLabel)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	QGroupBox *videoInformationGroupBox = new QGroupBox(QString("Video information"), this);

	QVBoxLayout *videoInformationLayout = new QVBoxLayout(videoInformationGroupBox);
	videoInformationLayout->addWidget(videoFilenameLabel);
	videoInformationLayout->addWidget(frameRateLabel);

	layout->addWidget(videoInformationGroupBox);
}

void RealLiveVideoWidget::newRealLiveVideo(RealLiveVideo &realLiveVideo)
{
	videoFilenameLabel->setText(realLiveVideo.videoInformation.videoFilename);
	frameRateLabel->setNum(realLiveVideo.videoInformation.frameRate);
}
