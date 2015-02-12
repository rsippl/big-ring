#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtCore/QSettings>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>

class PreferencesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = 0);

    virtual QSize sizeHint() const override;
private:
    void setUpUnitPreferences(QFormLayout* layout, QSettings &settings);
};

#endif // PREFERENCESDIALOG_H
