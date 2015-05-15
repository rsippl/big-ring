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

#include <QtCore/QMap>
#include <QtCore/QSettings>
#include <QtCore/QtDebug>

#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

using indoorcycling::AntCentralDispatch;

namespace {

const char* SEARCHING = "Searching";
const char* FOUND = "Found";
const char* NOT_FOUND = "Not Found";

int sensorTypeRole = Qt::UserRole + 1;
int sensorDeviceNumberRole = sensorTypeRole + 1;

enum class SearchTableColumn {
    NAME,
    BUTTON,
    STATE,
    DEVICE_NUMBER,
    VALUE
};
int columnNumber(SearchTableColumn column) {
    return static_cast<int>(column);
}
}

SettingsDialog::SettingsDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                               QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog),
    _antCentralDispatch(antCentralDispatch)
{
    _ui->setupUi(this);
    _ui->searchTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_HR);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_POWER);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED);
    QSettings settings;
    _ui->unitChooser->setCurrentText(settings.value("units").toString());
    _ui->weightSpinBox->setValue(settings.value("cyclist.weight", QVariant::fromValue(82)).toInt());
    _ui->robotCheckBox->setChecked(settings.value("useRobot", QVariant::fromValue(false)).toBool());
    _ui->powerSpinBox->setValue(settings.value("robotPower", QVariant::fromValue(250)).toInt());
    _ui->powerSpinBox->setEnabled(_ui->robotCheckBox->isChecked());

    fillUsbStickPresentLabel(_antCentralDispatch->antUsbStickPresent());
    connect(_antCentralDispatch, &AntCentralDispatch::antUsbStickScanningFinished, this,
            &SettingsDialog::fillUsbStickPresentLabel);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this,
            &SettingsDialog::sensorFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorNotFound, this,
            &SettingsDialog::sensorNotFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorValue, this,
            &SettingsDialog::handleSensorValue);
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

void SettingsDialog::on_searchSensorsButton_clicked()
{
    _antCentralDispatch->closeAllChannels();
    _ui->searchSensorsButton->setEnabled(false);
    performSearch(indoorcycling::SENSOR_TYPE_HR);
    performSearch(indoorcycling::SENSOR_TYPE_POWER);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED);
}

void SettingsDialog::sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        _currentSearches.remove(sensorType);
        _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::BUTTON))
                ->setEnabled(true);
        QProgressBar* bar = static_cast<QProgressBar*>(
                    _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::STATE)));
        bar->setMaximum(10);
        bar->setFormat(tr(FOUND));
        QTableWidgetItem* const deviceNumberItem =
                _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::DEVICE_NUMBER));
        deviceNumberItem->setText(QString::number(deviceNumber));
        _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::NAME))
                ->setData(sensorDeviceNumberRole, QVariant::fromValue(deviceNumber));
    }
    if (_currentSearches.isEmpty()) {
        _ui->searchSensorsButton->setEnabled(true);
    }
}

void SettingsDialog::sensorNotFound(indoorcycling::AntSensorType sensorType)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        _currentSearches.remove(sensorType);
        _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::BUTTON))
                ->setEnabled(true);
        QProgressBar* bar = static_cast<QProgressBar*>(
                    _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::STATE)));
        bar->setMaximum(10);
        bar->setFormat(tr(NOT_FOUND));
        QTableWidgetItem* const deviceNumberItem =
                _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::DEVICE_NUMBER));
        deviceNumberItem->setText("-");
    }
    if (_currentSearches.isEmpty()) {
        _ui->searchSensorsButton->setEnabled(true);
    }
}

void SettingsDialog::handleSensorValue(const indoorcycling::SensorValueType sensorValueType,
                                       const indoorcycling::AntSensorType sensorType,
                                       const QVariant &sensorValue)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        QTableWidgetItem* item = _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::VALUE));
        QString text = QString("%1 %2").arg(QString::number(sensorValue.toInt()))
                .arg(indoorcycling::SENSOR_VALUE_TYPE_STRINGS[sensorValueType]);
        item->setText(text);
    }
}

void SettingsDialog::performSearch(indoorcycling::AntSensorType sensorType)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        // disable push button
        _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::BUTTON))
                ->setEnabled(false);
        // start search bar
        QProgressBar* bar = static_cast<QProgressBar*>(
                    _ui->searchTableWidget->cellWidget(row, columnNumber(SearchTableColumn::STATE)));
        bar->setRange(0, 0);
        bar->setFormat(tr(SEARCHING));
        bar->setTextVisible(true);
        bar->setValue(0);

        _currentSearches.insert(sensorType);
        _antCentralDispatch->searchForSensorType(sensorType);

    }
}

void SettingsDialog::fillSensorTypeRow(indoorcycling::AntSensorType sensorType)
{
    _currentSearches.insert(sensorType);
    int row = _ui->searchTableWidget->rowCount();
    _ui->searchTableWidget->setRowCount(row + 1);
    QTableWidgetItem* nameColumn = new QTableWidgetItem(
                indoorcycling::ANT_SENSOR_TYPE_STRINGS[sensorType]);
    nameColumn->setData(sensorTypeRole, QVariant::fromValue(static_cast<int>(sensorType)));
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::NAME), nameColumn);
    QPushButton* button = new QPushButton(tr("Search"));
    connect(button, &QPushButton::clicked, button, [this, sensorType]() {
        performSearch(sensorType);
    });
    _ui->searchTableWidget->setCellWidget(row, columnNumber(SearchTableColumn::BUTTON), button);
    QProgressBar* bar = new QProgressBar;
    bar->setRange(0, 0);
    bar->setMaximum(10);
    bar->setFormat(tr(NOT_FOUND));
    _ui->searchTableWidget->setCellWidget(row, columnNumber(SearchTableColumn::STATE), bar);

    QTableWidgetItem* deviceNumberColumn = new QTableWidgetItem("-");
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::DEVICE_NUMBER),
                                    deviceNumberColumn);
    QTableWidgetItem* currentValueColumn = new QTableWidgetItem("-");
    _ui->searchTableWidget->setItem(row, columnNumber(SearchTableColumn::VALUE),
                                    currentValueColumn);
}

int SettingsDialog::rowForSensorType(indoorcycling::AntSensorType typeToFind)
{
    for (int row = 0; row < _ui->searchTableWidget->rowCount(); ++row) {
        QTableWidgetItem* nameItem = _ui->searchTableWidget->item(row, 0);
        indoorcycling::AntSensorType sensorType =
                static_cast<indoorcycling::AntSensorType>(nameItem->data(sensorTypeRole).toInt());
        if (sensorType == typeToFind) {
            return row;
        }
    }
    return -1;
}

void SettingsDialog::on_pushButton_clicked()
{
    QSettings settings;

    settings.beginGroup("Sensor_Configurations");
    settings.beginGroup("Sensor_Configuration_1");
    settings.beginWriteArray("sensors");
    int settingsIndex = 0;
    for (int row = 0; row < _ui->searchTableWidget->rowCount(); ++row) {
        QTableWidgetItem* nameItem = _ui->searchTableWidget->item(row, 0);
        indoorcycling::AntSensorType sensorType =
                static_cast<indoorcycling::AntSensorType>(nameItem->data(sensorTypeRole).toInt());
        QVariant sensorDeviceNumber =
                nameItem->data(sensorDeviceNumberRole);
        if (!sensorDeviceNumber.isNull()) {
            settings.setArrayIndex(settingsIndex++);
            settings.setValue("sensorType", QVariant::fromValue(static_cast<int>(sensorType)));
            settings.setValue("deviceNumber", QVariant::fromValue(sensorDeviceNumber));
        }
    }

    settings.endArray();
    settings.endGroup();
    settings.setValue("selectedConfiguration", "Sensor_Configuration_1");
    settings.endGroup();
}
