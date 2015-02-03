#include "videodetailswidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>

#include "thumbnailer.h"
#include "videoscreenshotlabel.h"

VideoDetailsWidget::VideoDetailsWidget(QWidget *parent) :
    QWidget(parent), _thumbnailer(new Thumbnailer(this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addWidget(setupVideoScreenshot());
    layout->addWidget(setupDetails());

    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, _thumbnailer, [this](const RealLifeVideo& rlv, QPixmap pixmap){
        updateVideoScreenshotLabel(rlv, pixmap);
    });
}

void VideoDetailsWidget::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    _nameLabel->setText(rlv.name());
    _distanceLabel->setText(QString("%1 km").arg(QString::number(rlv.totalDistance() / 1000, 'f', 1)));
    qDebug() << "label size" << _videoScreenshotLabel->size();
    _videoScreenshotLabel->setPixmap(_thumbnailer->thumbnailFor(rlv));
}

QWidget *VideoDetailsWidget::setupDetails()
{
    QGroupBox* detailsGroupBox = new QGroupBox("Details", this);
    QVBoxLayout* layout = new QVBoxLayout(this);

    _nameLabel = new QLabel;
    layout->addWidget(_nameLabel);
    _distanceLabel = new QLabel;
    layout->addWidget(_distanceLabel);

    detailsGroupBox->setLayout(layout);
    return detailsGroupBox;
}

QWidget *VideoDetailsWidget::setupVideoScreenshot()
{
    _videoScreenshotLabel = new VideoScreenshotLabel;
    _videoScreenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
//    _videoScreenshotLabel->setScaledContents(true);

    return _videoScreenshotLabel;
}

void VideoDetailsWidget::updateVideoScreenshotLabel(const RealLifeVideo &rlv, QPixmap &pixmap)
{
    qDebug() << "updated pixmap" << rlv.name();
    RealLifeVideo thisRlv = rlv;
    if (thisRlv == _currentRlv) {
        _videoScreenshotLabel->setPixmap(pixmap);
    }
}
