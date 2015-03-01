#include "videodetails.h"
#include "ui_videodetails.h"
#include "quantityprinter.h"
#include "thumbnailer.h"

VideoDetails::VideoDetails(QWidget *parent) :
    QWidget(parent),
    _thumbnailer(new Thumbnailer(this)),
    _quantityPrinter(new QuantityPrinter(this)),
    ui(new Ui::VideoDetails)
{
    ui->setupUi(this);

    connect(_thumbnailer, &Thumbnailer::pixmapUpdated, _thumbnailer, [this](const RealLifeVideo& rlv, const qreal distance, QPixmap pixmap){
        updateVideoScreenshotLabel(rlv, distance, pixmap);
    });
}

VideoDetails::~VideoDetails()
{
    delete ui;
}

void VideoDetails::setVideo(RealLifeVideo &rlv)
{
    _currentRlv = rlv;
    ui->nameLabel->setText(rlv.name());

    ui->distanceLabel->setText(QString("%1 %2").arg(_quantityPrinter->printDistance(rlv.totalDistance())).arg(_quantityPrinter->unitString(QuantityPrinter::Distance)));
    ui->videoScreenshotWidget->setPixmap(_thumbnailer->thumbnailFor(rlv));
    ui->altitudeProfileWidget->setVideo(rlv);

    ui->courseListWidget->clear();
    for (const Course& course: rlv.courses()) {
        new QListWidgetItem(course.name(), ui->courseListWidget);
    }
    ui->courseListWidget->setCurrentRow(0);
}

void VideoDetails::updateVideoScreenshotLabel(const RealLifeVideo &rlv, const qreal distance, QPixmap &pixmap)
{
    RealLifeVideo thisRlv = rlv;
    if (thisRlv == _currentRlv && _courseIndex >= 0 && qFuzzyCompare(distance, static_cast<qreal>(_currentRlv.courses()[_courseIndex].start()))) {
        ui->videoScreenshotWidget->setPixmap(pixmap);
    }
}

void VideoDetails::on_pushButton_clicked()
{
    emit playClicked(_currentRlv, ui->courseListWidget->currentRow());
}

void VideoDetails::on_courseListWidget_currentRowChanged(int currentRow)
{
    _courseIndex = currentRow;
    ui->altitudeProfileWidget->setCourseIndex(_courseIndex);
    if (currentRow >= 0) {
        ui->videoScreenshotWidget->setPixmap(_thumbnailer->thumbnailFor(_currentRlv, _currentRlv.courses()[currentRow].start()));
        qDebug() << "course selected:" << _currentRlv.courses()[currentRow].name();
    }
}
