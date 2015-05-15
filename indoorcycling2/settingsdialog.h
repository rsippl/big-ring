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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtWidgets/QDialog>

#include "antcentraldispatch.h"
#include "antsensortype.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                            QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void on_unitChooser_currentTextChanged(const QString &arg1);

    void on_robotCheckBox_toggled(bool checked);

    void on_powerSpinBox_valueChanged(int arg1);

    void on_weightSpinBox_valueChanged(int arg1);

    void fillUsbStickPresentLabel(bool present);
    void on_searchSensorsButton_clicked();
    void sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber);
    void sensorNotFound(indoorcycling::AntSensorType sensorType);
    void handleSensorValue(const indoorcycling::SensorValueType sensorValueType,
                     const indoorcycling::AntSensorType sensorType,
                     const QVariant& sensorValue);
    void performSearch(indoorcycling::AntSensorType sensorType);
private:
    void fillSensorTypeRow(indoorcycling::AntSensorType);
    int rowForSensorType(indoorcycling::AntSensorType);

    Ui::SettingsDialog *_ui;
    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
    QSet<indoorcycling::AntSensorType> _currentSearches;
};

#endif // SETTINGSDIALOG_H
