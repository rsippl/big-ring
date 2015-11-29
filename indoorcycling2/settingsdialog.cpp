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

#include "bigringsettings.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"


#include "addsensorconfigurationdialog.h"

#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QtDebug>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

using indoorcycling::AntCentralDispatch;
using indoorcycling::SimulationSetting;

namespace {

const char* SENSOR_PRESENT = "Present (device number %1)";

const char* FIXED_POWER_LABEL = "No power measurement. Power fixed to %1 watts.\n"
        "Road Speed is calculated using this power.";
const char* DIRECT_POWER_LABEL = "Power measured using power meter."
        "Road Speed is calculated using this power.";
const char* DIRECT_SPEED_LABEL = "Speed measured using speed sensor."
        "This speed is used directly as road speed.";
const char* VIRTUAL_POWER_LABEL = "Power derived from speed measured using speed sensor and power curve of trainer."
        "Road speed is calculated using this power.";

}

SettingsDialog::SettingsDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                               std::function<void(void)> &videoLoadFunction,
                               QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog),
    _antCentralDispatch(antCentralDispatch),
    _videoLoadFunction(videoLoadFunction)
{
    _ui->setupUi(this);
    QSettings settings;
    _ui->unitChooser->setCurrentText(settings.value("units").toString());
    BigRingSettings bigRingSettings;
    _ui->userWeightSpinBox->setValue(bigRingSettings.userWeight());
    _ui->bikeWeightSpinBox->setValue(bigRingSettings.bikeWeight());

    reset();
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

void SettingsDialog::on_pushButton_clicked()
{
    AddSensorConfigurationDialog dialog(_antCentralDispatch, this);
    dialog.exec();
    reset();
}

void SettingsDialog::fillSensorSettingsComboBox()
{
    _ui->antConfigurationChooser->blockSignals(true);
    _ui->antConfigurationChooser->clear();
    QMap<QString,indoorcycling::NamedSensorConfigurationGroup> configurationGroups =
            indoorcycling::NamedSensorConfigurationGroup::readFromSettings();
    _ui->antConfigurationChooser->addItems(configurationGroups.keys());

    const QString currentConfigurationName =
            indoorcycling::NamedSensorConfigurationGroup::selectedConfigurationGroup().name();

    if (!configurationGroups.isEmpty()) {
        _ui->deleteConfigurationButton->setEnabled(true);
        if (configurationGroups.contains(currentConfigurationName)) {
            _ui->antConfigurationChooser->setCurrentText(currentConfigurationName);
        } else {
            _ui->antConfigurationChooser->setCurrentIndex(0);
            indoorcycling::NamedSensorConfigurationGroup::saveSelectedConfigurationGroup(
                        configurationGroups.first().name());
        }
    } else {
        _ui->deleteConfigurationButton->setEnabled(false);
    }
    _ui->antConfigurationChooser->blockSignals(false);
}

void SettingsDialog::fillSensorLabels()
{
    auto configurationGroup =
            indoorcycling::NamedSensorConfigurationGroup::selectedConfigurationGroup();
    auto configurations = configurationGroup.sensorConfigurations();
    fillSensorLabel(_ui->hrSensorLabel, configurations,
                    indoorcycling::AntSensorType::HEART_RATE);
    fillSensorLabel(_ui->cadenceSensorLabel, configurations,
                    indoorcycling::AntSensorType::CADENCE);
    fillSensorLabel(_ui->speedSensorLabel, configurations,
                    indoorcycling::AntSensorType::SPEED);
    fillSensorLabel(_ui->speedAndCadenceSensorLabel, configurations,
                    indoorcycling::AntSensorType::SPEED_AND_CADENCE);
    fillSensorLabel(_ui->powerSensorLabel, configurations,
                    indoorcycling::AntSensorType::POWER);
    fillSensorLabel(_ui->smartTrainerLabel, configurations,
                    indoorcycling::AntSensorType::SMART_TRAINER);

}

void SettingsDialog::fillSensorLabel(QLabel* label,
                                     const QMap<indoorcycling::AntSensorType,
                                        indoorcycling::SensorConfiguration>& configurations,
                                     const indoorcycling::AntSensorType type)
{
    if (configurations.contains(type)) {
        int deviceNumber = configurations[type].deviceNumber();
        label->setText(tr(SENSOR_PRESENT).arg(deviceNumber));
    } else {
        label->setText("-");
    }
}

void SettingsDialog::fillSimulationSettingLabel()
{
    auto configurationGroup =
            indoorcycling::NamedSensorConfigurationGroup::selectedConfigurationGroup();
    switch(configurationGroup.simulationSetting()) {
    case SimulationSetting::FIXED_POWER:
        _ui->simulationSettingLabel->setText(tr(FIXED_POWER_LABEL).arg(configurationGroup.fixedPower()));
        break;
    case SimulationSetting::VIRTUAL_POWER:
        _ui->simulationSettingLabel->setText(tr(VIRTUAL_POWER_LABEL));
        break;
    case SimulationSetting::DIRECT_POWER:
        _ui->simulationSettingLabel->setText(tr(DIRECT_POWER_LABEL));
        break;
    case SimulationSetting::DIRECT_SPEED:
        _ui->simulationSettingLabel->setText(tr(DIRECT_SPEED_LABEL));
        break;
    }
}

void SettingsDialog::fillVideoFolderList()
{
    _ui->videoFolderLineEdit->setText(_settings.videoFolder());
}

void SettingsDialog::fillPowerAveragingComboBox()
{
    _ui->powerAveragingCombobox->blockSignals(true);

    _ui->powerAveragingCombobox->addItem(tr("No Averaging"), 0);
    _ui->powerAveragingCombobox->addItem(tr("1 Second"), 1000);
    _ui->powerAveragingCombobox->addItem(tr("3 Seconds"), 3000);
    _ui->powerAveragingCombobox->addItem(tr("10 Seconds"), 10000);

    bool found = false;

    int value = _settings.powerAveragingForDisplayMilliseconds();
    for (int i = 0; i < _ui->powerAveragingCombobox->count(); ++i) {
        if (_ui->powerAveragingCombobox->itemData(i).toInt() == value) {
            _ui->powerAveragingCombobox->setCurrentIndex(i);
            found = true;
        }
    }
    _ui->powerAveragingCombobox->blockSignals(false);
    if (!found) {
        _ui->powerAveragingCombobox->setCurrentIndex(0);
    }
}

void SettingsDialog::saveVideoFolder(const QString& folder)
{
    BigRingSettings().setVideoFolder(folder);
    _videoLoadFunction();
}

void SettingsDialog::on_antConfigurationChooser_currentIndexChanged(
        const QString &selectedConfiguration)
{
    indoorcycling::NamedSensorConfigurationGroup::saveSelectedConfigurationGroup(
                selectedConfiguration);
    reset();
}

void SettingsDialog::reset()
{
    fillSensorSettingsComboBox();
    fillSensorLabels();
    fillSimulationSettingLabel();
    fillVideoFolderList();
    fillPowerAveragingComboBox();
}

void SettingsDialog::on_deleteConfigurationButton_clicked()
{
    QString configurationName = _ui->antConfigurationChooser->currentText();
    int result = QMessageBox::warning(this, tr("Remove Sensor Configuration?"),
                                      tr("Sensor Configuration %1 will be removed.\n"
                                         "Are you sure?").arg(configurationName),
                                      QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (result == QMessageBox::Ok) {
        indoorcycling::NamedSensorConfigurationGroup::removeConfigurationGroup(
                    configurationName);
    }
    reset();
}

void SettingsDialog::on_changeFolderButton_clicked()
{
    QStringList homeDirectories = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);

    QString startDirectory = (homeDirectories.isEmpty()) ? QString() : homeDirectories[0];
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open RLV Directory"),
                                                    startDirectory);

    saveVideoFolder(dir);
    fillVideoFolderList();
}

void SettingsDialog::on_powerAveragingCombobox_currentIndexChanged(int index)
{
    QVariant data = _ui->powerAveragingCombobox->itemData(index);
    // values are 1, 3 and 10 seconds.
    _settings.setPowerAveragingForDisplayMilliseconds(data.toInt());

}

void SettingsDialog::on_userWeightSpinBox_valueChanged(double userWeight)
{
    BigRingSettings().setUserWeight(userWeight);
}

void SettingsDialog::on_bikeWeightSpinBox_valueChanged(double bikeWeight)
{
    BigRingSettings().setBikeWeight(bikeWeight);
}
