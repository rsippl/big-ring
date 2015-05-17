#ifndef SENSORS_H
#define SENSORS_H

#include <QtCore/QObject>

#include "antcentraldispatch.h"
#include "sensorconfiguration.h"

namespace indoorcycling
{
class Sensors : public QObject
{
    Q_OBJECT
public:
    explicit Sensors(AntCentralDispatch* antCentralDispatch, QObject *parent = 0);

signals:
    void heartRateBpmMeasured(int heartRate);
    void powerWattsMeasured(int power);
    void cadenceRpmMeasured(float cadence);
    /** wheel speed in revolutions per minute */
    void wheelSpeedRpmMeasured(float speed);
    /** wheel speed in m/s */
    void wheelSpeedMpsMeasured(float speed);
public slots:
    void initialize();
private slots:
    void sensorValue(const SensorValueType sensorValueType, const AntSensorType sensorType,
                     const QVariant& sensorValue);
    void sendPowerUpdate();
private:
    void handleHeartRate(const QVariant& sensorValue);
    void handleCadence(const QVariant& sensorValue);
    void handlePower(const QVariant& sensorValue);
    void handleWheelSpeed(const QVariant& sensorValue);
    void calculatePower(const float wheelSpeedRpm);

    AntCentralDispatch* const _antCentralDispatch;
    const NamedSensorConfigurationGroup _sensorConfigurationGroup;
    QTimer* _updateTimer;

    int _heartRateBpm;
    int _powerWatts;
    float _cadenceRpm;
    float _wheelSpeedRpm;
};
}
#endif // SENSORS_H
