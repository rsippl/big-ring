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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include "addsensorconfigurationdialog.h"

#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QtDebug>

using indoorcycling::AntCentralDispatch;

namespace {

const char* FOUND = "Found";
const char* NOT_FOUND = "Not Found";
}

SettingsDialog::SettingsDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                               QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog),
    _antCentralDispatch(antCentralDispatch)
{
    _ui->setupUi(this);
    QSettings settings;
    _ui->unitChooser->setCurrentText(settings.value("units").toString());
    _ui->weightSpinBox->setValue(settings.value("cyclist.weight", QVariant::fromValue(82)).toInt());
    _ui->robotCheckBox->setChecked(settings.value("useRobot", QVariant::fromValue(false)).toBool());
    _ui->powerSpinBox->setValue(settings.value("robotPower", QVariant::fromValue(250)).toInt());
    _ui->powerSpinBox->setEnabled(_ui->robotCheckBox->isChecked());

    fillUsbStickPresentLabel(_antCentralDispatch->antUsbStickPresent());
    connect(_antCentralDispatch, &AntCentralDispatch::antUsbStickScanningFinished, this,
            &SettingsDialog::fillUsbStickPresentLabel);
    fillSensorSettingsComboBox();
}

SettingsDialog::~SettingsDialog()
{
    delete _ui;
}

void SettingsDialog::on_unitChooser_currentTextChanged(const QString &choice)
{
    QSettings settings;
    settings.setValue("units", choice);
}

void SettingsDialog::on_robotCheckBox_toggled(bool checked)
{
    QSettings settings;
    settings.setValue("useRobot", QVariant::fromValue(checked));
    settings.setValue("robotPower", _ui->powerSpinBox->value());
}

void SettingsDialog::on_powerSpinBox_valueChanged(int robotPower)
{
    QSettings settings;
    settings.setValue("robotPower", QVariant::fromValue(robotPower));
}

void SettingsDialog::on_weightSpinBox_valueChanged(int cyclistWeight)
{
    QSettings settings;
    settings.setValue("cyclist.weight", QVariant::fromValue(cyclistWeight));
}

void SettingsDialog::fillUsbStickPresentLabel(bool present)
{
    QString text = tr((present) ? FOUND : NOT_FOUND);
    _ui->usbStickPresentLabel->setText(QString("<b>%1</b>").arg(text));
}

void SettingsDialog::on_pushButton_clicked()
{
    AddSensorConfigurationDialog dialog(_antCentralDispatch, this);
    dialog.exec();
}

void SettingsDialog::fillSensorSettingsComboBox()
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.beginGroup("configurations");
    const QStringList configurationNames = settings.childGroups();
    _ui->antConfigurationChooser->addItems(configurationNames);
    settings.endGroup();
    const QString currentConfiguration = settings.value("selectedConfiguration").toString();
    if (!configurationNames.isEmpty()) {
        if (currentConfiguration.isNull() || !configurationNames.contains(currentConfiguration)) {
            _ui->antConfigurationChooser->setCurrentIndex(0);
        } else {
            _ui->antConfigurationChooser->setCurrentText(currentConfiguration);
        }
    }
    settings.endGroup();
}

void SettingsDialog::on_antConfigurationChooser_currentIndexChanged(
        const QString &selectedConfiguration)
{
    QSettings settings;
    settings.beginGroup("Sensor_Configuration");
    settings.setValue("selectedConfiguration", QVariant::fromValue(selectedConfiguration));
}
