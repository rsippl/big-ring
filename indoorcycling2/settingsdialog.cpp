#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtCore/QSettings>
#include <QtCore/QtDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::SettingsDialog)
{
    _ui->setupUi(this);

    QSettings settings;
    _ui->unitChooser->setCurrentText(settings.value("units").toString());
    _ui->weightSpinBox->setValue(settings.value("cyclist.weight", QVariant::fromValue(82)).toInt());
    _ui->robotCheckBox->setChecked(settings.value("useRobot", QVariant::fromValue(false)).toBool());
    _ui->powerSpinBox->setValue(settings.value("robotPower", QVariant::fromValue(250)).toInt());
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
    _ui->powerSpinBox->setEnabled(checked);
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
