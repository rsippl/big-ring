#include "preferencesdialog.h"

#include <QtCore/QSettings>
#include <QtCore/QtDebug>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Preferences"));

    QFormLayout* layout = new QFormLayout;
    setLayout(layout);

    QSettings settings;
    setUpUnitPreferences(layout, settings);
}

QSize PreferencesDialog::sizeHint() const
{
    return QSize(400, 200);
}

void PreferencesDialog::setUpUnitPreferences(QFormLayout *layout, QSettings& settings)
{
    QLabel* label = new QLabel(tr("Units"));

    QComboBox* unitsComboBox = new QComboBox();
    label->setBuddy(unitsComboBox);

    QStringList unitSettings({"Metric", "Imperial"});
    QString fromSettings = settings.value("units").toString();
    unitsComboBox->addItems(unitSettings);
    if (unitSettings.contains(fromSettings)) {
        unitsComboBox->setCurrentText(fromSettings);
    }

    connect(unitsComboBox, &QComboBox::currentTextChanged, unitsComboBox, [=](const QString& choice) {
        QSettings settings;
        settings.setValue("units", choice);
        qDebug() << "choosen units:" << choice;
    });
    layout->addRow(label, unitsComboBox);
}
