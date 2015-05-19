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
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>

#include "antcentraldispatch.h"
#include "antsensortype.h"
#include "sensorconfiguration.h"

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

    void on_weightSpinBox_valueChanged(int arg1);

    void fillUsbStickPresentLabel(bool present);
    void on_pushButton_clicked();

    void on_antConfigurationChooser_currentIndexChanged(const QString &selectedConfiguration);

    void on_deleteConfigurationButton_clicked();

private:
    void reset();
    void fillSensorSettingsComboBox();
    void fillSensorLabels();
    void fillSensorLabel(QLabel* label,
                         const QMap<indoorcycling::AntSensorType, indoorcycling::SensorConfiguration> &configurations,
                         const indoorcycling::AntSensorType type);
    void fillSimulationSettingLabel();

    Ui::SettingsDialog *_ui;
    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
};

#endif // SETTINGSDIALOG_H