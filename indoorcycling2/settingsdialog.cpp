#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QtCore/QSettings>
#include <QtCore/QtDebug>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    QSettings settings;
    ui->unitChooser->setCurrentText(settings.value("units").toString());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_unitChooser_currentTextChanged(const QString &choice)
{
    QSettings settings;
    settings.setValue("units", choice);
    qDebug() << "choosen units:" << choice;
}
