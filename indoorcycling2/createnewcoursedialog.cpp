#include "createnewcoursedialog.h"
#include "ui_createnewcoursedialog.h"

#include <QtWidgets/QPushButton>

#include "thumbnailer.h"
#include "quantityprinter.h"

CreateNewCourseDialog::CreateNewCourseDialog(RealLifeVideo &rlv, QWidget *parent) :
    QDialog(parent), ui(new Ui::CreateNewCourseDIalog),
    _thumbnailer(new Thumbnailer(this)), _quantityPrinter(new QuantityPrinter(this)),
    _rlv(rlv), _startDistanceInMeters(0)
{
    ui->setupUi(this);

    ui->startDistanceSlider->setMaximum(static_cast<int>(rlv.totalDistance()));
    ui->startDistanceSlider->setValue(_startDistanceInMeters);

    ui->videoScreenshotWidget->setVideo(rlv);
    ui->altitudeProfileWidget->setVideo(rlv);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

CreateNewCourseDialog::~CreateNewCourseDialog()
{
    delete ui;
}

int CreateNewCourseDialog::startDistanceInMeters() const
{
    return _startDistanceInMeters;
}

const QString CreateNewCourseDialog::courseName() const
{
    return ui->courseNameEdit->text();
}

void CreateNewCourseDialog::on_startDistanceSlider_valueChanged(int value)
{
    _startDistanceInMeters = value;
    ui->distanceLabel->setText(QString("%1 %2").arg(_quantityPrinter->printDistance(_startDistanceInMeters))
                               .arg(_quantityPrinter->unitString(QuantityPrinter::Distance)));
    ui->altitudeProfileWidget->setStartAndEndDistance(_startDistanceInMeters, _rlv.totalDistance());
}

void CreateNewCourseDialog::on_startDistanceSlider_sliderReleased()
{
    ui->videoScreenshotWidget->setDistance(_startDistanceInMeters);
}

void CreateNewCourseDialog::on_courseNameEdit_textChanged(const QString &courseName)
{
    if (!courseName.isEmpty()) {
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    }
}
