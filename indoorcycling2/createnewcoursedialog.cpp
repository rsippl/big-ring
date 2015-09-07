/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
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
                               .arg(_quantityPrinter->unitString(QuantityPrinter::Quantity::Distance)));
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
