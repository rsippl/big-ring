
#include "actuators.h"
#include "cyclist.h"

#include "antlib/antcentraldispatch.h"
namespace indoorcycling {

indoorcycling::Actuators::Actuators(const Cyclist *cyclist, indoorcycling::AntCentralDispatch *antCentralDispatch,
                                    const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent):
    QObject(parent), _cyclist(cyclist), _antCentralDispatch(antCentralDispatch), _sensorConfigurationGroup(sensorConfigurationGroup)
{
    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this, &Actuators::setSensorFound);
}

void Actuators::initialize()
{

}

void Actuators::setSensorFound(AntSensorType channelType, int)
{
    if (channelType == AntSensorType::SMART_TRAINER) {
        configureWeight();
    }
}

void Actuators::setSlope(qreal slopeInPercent)
{
    _antCentralDispatch->setSlope(slopeInPercent);
}

void Actuators::configureWeight()
{
    _antCentralDispatch->setWeight(_cyclist->weight(), 10.0);
}
}
