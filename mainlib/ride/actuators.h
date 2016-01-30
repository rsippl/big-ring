#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <QtCore/QObject>
#include "config/sensorconfiguration.h"

class Cyclist;
namespace indoorcycling {

class AntCentralDispatch;
class Actuators : public QObject
{
    Q_OBJECT
public:
    explicit Actuators(const Cyclist* cyclist, AntCentralDispatch *antCentralDispatch, const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent = nullptr);
public slots:
    void initialize();
    void setSensorFound(AntSensorType channelType, int deviceNumber);
    void setSlope(qreal slopeInPercent);

private:
    void configureWeight();

    const Cyclist *const _cyclist;
    AntCentralDispatch* const _antCentralDispatch;
    const NamedSensorConfigurationGroup _sensorConfigurationGroup;

    qreal _currentSlope = 0.0;
    const qreal _maximumSlope;
    const qreal _minimumSlope;
};
}
#endif // ACTUATORS_H
