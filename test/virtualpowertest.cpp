#include "virtualpowertest.h"

#include "common.h"
#include "virtualpower.h"

using namespace indoorcycling;
void VirtualPowerTest::testKurtKineticRoadMachine()
{
    VirtualPowerFunctionType kkFunction = virtualPowerFunctionForTrainer(
                VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE);

    float speedMph = 16.1f;
    float speedMps = speedMph / 3600 * 1609.344;
    QCOMPARE(kkFunction(speedMps), 164.435f);
    QCOMPARE(static_cast<int>(kkFunction(7.964800)), 201);
    QCOMPARE(kkFunction(7.964800), 7.964800f);
}
