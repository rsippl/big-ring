
#include "actuators.h"
#include "model/cyclist.h"

#include "ant/antcentraldispatch.h"
namespace indoorcycling {

Actuators::Actuators(const Cyclist *cyclist, indoorcycling::AntCentralDispatch *antCentralDispatch,
                                    const NamedSensorConfigurationGroup &sensorConfigurationGroup, QObject *parent):
    QObject(parent), _cyclist(cyclist), _antCentralDispatch(antCentralDispatch), _sensorConfigurationGroup(sensorConfigurationGroup)
{
    connect(_antCentralDispatch, &AntCentralDispatch::sensorFound, this, &Actuators::setSensorFound);
}

void Actuators::initialize()
{
    // empty
}

void Actuators::setSensorFound(AntSensorType channelType, int)
{
    if (channelType == AntSensorType::SMART_TRAINER) {
        configureWeight();
    }
}

void Actuators::setSlope(qreal slopeInPercent)
{
    if (!qFuzzyCompare(slopeInPercent, _currentSlope)) {
        _currentSlope = slopeInPercent;
        _antCentralDispatch->setSlope(slopeInPercent);
    }
}

void Actuators::configureWeight()
{
    _antCentralDispatch->setWeight(_cyclist->userWeight(), _cyclist->bikeWeight());
}
}
