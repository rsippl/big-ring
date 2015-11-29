#ifndef ACTUATORS_H
#define ACTUATORS_H

#include <QObject>
#include "sensorconfiguration.h"

namespace indoorcycling {

class AntCentralDispatch;
class Actuators : public QObject
{
    Q_OBJECT
public:
    explicit Actuators(AntCentralDispatch *antCentralDispatch, const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent = nullptr);
public slots:
    void initialize();
    void setSlope(qreal slopeInPercent);
private:
    AntCentralDispatch* const _antCentralDispatch;
    const NamedSensorConfigurationGroup _sensorConfigurationGroup;
};
}
#endif // ACTUATORS_H
