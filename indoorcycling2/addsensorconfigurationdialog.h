#ifndef ADDSENSORCDIALOG_H
#define ADDSENSORCDIALOG_H

#include <QtCore/QSet>
#include <QtWidgets/QDialog>

#include "antcentraldispatch.h"
#include "antsensortype.h"

namespace Ui {
class AddSensorConfigurationDialog;
}

class AddSensorConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddSensorConfigurationDialog(indoorcycling::AntCentralDispatch* antCentralDispatch,
                                          QWidget *parent = 0);
    ~AddSensorConfigurationDialog();

private slots:
    void on_searchSensorsButton_clicked();

    void sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber);
    void sensorNotFound(indoorcycling::AntSensorType sensorType);
    void handleSensorValue(const indoorcycling::SensorValueType sensorValueType,
                     const indoorcycling::AntSensorType sensorType,
                     const QVariant& sensorValue);
    void performSearch(indoorcycling::AntSensorType sensorType);
private:
    void fillSensorTypeRow(indoorcycling::AntSensorType sensorType);
    int rowForSensorType(indoorcycling::AntSensorType);

    Ui::AddSensorConfigurationDialog *_ui;
    indoorcycling::AntCentralDispatch* const _antCentralDispatch;
    QSet<indoorcycling::AntSensorType> _currentSearches;
};

#endif // ADDSENSORCDIALOG_H
