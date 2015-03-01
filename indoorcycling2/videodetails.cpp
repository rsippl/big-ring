#include "videodetails.h"
#include "ui_videodetails.h"
#include <QtCore/QtDebug>
#include "createnewcoursedialog.h"
#include "quantityprinter.h"

VideoDetails::VideoDetails(QWidget *parent) :
    QWidget(parent),
    _quantityPrinter(new QuantityPrinter(this)),
    ui(new Ui::VideoDetails)
{
    ui->setupUi(this);
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
    ui->videoScreenshotWidget->setVideo(rlv);
    ui->altitudeProfileWidget->setVideo(rlv);

    ui->courseListWidget->clear();
    for (const Course& course: rlv.courses()) {
        new QListWidgetItem(course.name(), ui->courseListWidget);
    }
    ui->courseListWidget->setCurrentRow(0);
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
        ui->videoScreenshotWidget->setDistance(_currentRlv.courses()[currentRow].start());
        qDebug() << "course selected:" << _currentRlv.courses()[currentRow].name();
    }
}

void VideoDetails::on_newCourseButton_clicked()
{
    CreateNewCourseDialog createNewCourseDialog(_currentRlv);
    int result = createNewCourseDialog.exec();
    if (result == QDialog::Accepted) {
        int startDistanceOfCustomRun = createNewCourseDialog.startDistanceInMeters();
        _currentRlv.addStartPoint(startDistanceOfCustomRun, createNewCourseDialog.courseName());

        QSettings settings;
        settings.beginGroup(QString("%1.custom_courses").arg(_currentRlv.name()));
        settings.setValue(createNewCourseDialog.courseName(), QVariant::fromValue(startDistanceOfCustomRun));
        settings.endGroup();

        ui->courseListWidget->clear();
        for (const Course& course: _currentRlv.courses()) {
            new QListWidgetItem(course.name(), ui->courseListWidget);
        }
        ui->courseListWidget->setCurrentRow(0);
    }
}
