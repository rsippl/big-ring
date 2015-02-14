#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtCore/QSettings>
#include <QtWidgets/QDialog>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

private slots:
    void on_unitChooser_currentTextChanged(const QString &arg1);

    void on_robotCheckBox_toggled(bool checked);

    void on_powerSpinBox_valueChanged(int arg1);

    void on_weightSpinBox_valueChanged(int arg1);

private:
    Ui::SettingsDialog *_ui;
};

#endif // SETTINGSDIALOG_H
