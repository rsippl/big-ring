#include "addsensorconfigurationdialog.h"
#include "ui_addsensorconfigurationdialog.h"

#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

using indoorcycling::AntCentralDispatch;

namespace
{
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
AddSensorConfigurationDialog::AddSensorConfigurationDialog(
        AntCentralDispatch* antCentralDispatch, QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::AddSensorConfigurationDialog),
    _antCentralDispatch(antCentralDispatch)
{
    _ui->setupUi(this);
    _ui->searchTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_HR);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_POWER);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_CADENCE);
    fillSensorTypeRow(indoorcycling::SENSOR_TYPE_SPEED);

    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this,
            &AddSensorConfigurationDialog::sensorFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorNotFound, this,
            &AddSensorConfigurationDialog::sensorNotFound);
    connect(_antCentralDispatch, &AntCentralDispatch::sensorValue, this,
            &AddSensorConfigurationDialog::handleSensorValue);

}

AddSensorConfigurationDialog::~AddSensorConfigurationDialog()
{
    delete _ui;
}

void AddSensorConfigurationDialog::fillSensorTypeRow(indoorcycling::AntSensorType sensorType)
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

int AddSensorConfigurationDialog::rowForSensorType(indoorcycling::AntSensorType typeToFind)
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

void AddSensorConfigurationDialog::on_searchSensorsButton_clicked()
{
    _antCentralDispatch->closeAllChannels();
    _ui->searchSensorsButton->setEnabled(false);
    performSearch(indoorcycling::SENSOR_TYPE_HR);
    performSearch(indoorcycling::SENSOR_TYPE_POWER);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED_AND_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_CADENCE);
    performSearch(indoorcycling::SENSOR_TYPE_SPEED);
}

void AddSensorConfigurationDialog::sensorFound(indoorcycling::AntSensorType sensorType, int deviceNumber)
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

void AddSensorConfigurationDialog::sensorNotFound(indoorcycling::AntSensorType sensorType)
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

void AddSensorConfigurationDialog::handleSensorValue(const indoorcycling::SensorValueType sensorValueType, const indoorcycling::AntSensorType sensorType, const QVariant &sensorValue)
{
    int row = rowForSensorType(sensorType);
    if (row >= 0) {
        QTableWidgetItem* item = _ui->searchTableWidget->item(row, columnNumber(SearchTableColumn::VALUE));
        QString text = QString("%1 %2").arg(QString::number(sensorValue.toInt()))
                .arg(indoorcycling::SENSOR_VALUE_TYPE_STRINGS[sensorValueType]);
        item->setText(text);
    }
}

void AddSensorConfigurationDialog::performSearch(indoorcycling::AntSensorType sensorType)
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
