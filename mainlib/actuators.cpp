
#include "actuators.h"

#include "antlib/antcentraldispatch.h"
namespace indoorcycling {

indoorcycling::Actuators::Actuators(indoorcycling::AntCentralDispatch *antCentralDispatch,
                                    const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent):
    QObject(parent), _antCentralDispatch(antCentralDispatch), _sensorConfigurationGroup(sensorConfigurationGroup)
{

}

void Actuators::initialize()
{
    // empty for now.
}

void Actuators::setSlope(qreal slopeInPercent)
{
    _antCentralDispatch->setSlope(slopeInPercent);
}
}
