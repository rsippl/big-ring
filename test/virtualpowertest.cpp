#include "virtualpowertest.h"

#include "common.h"
#include "mainlib/virtualpower.h"

using namespace indoorcycling;
void VirtualPowerTest::testKurtKineticRoadMachine()
{
    VirtualPowerFunctionType kkFunction = virtualPowerFunctionForTrainer(
                VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE);

    float speedMph = 16.1f;
    float speedMps = (speedMph / 3600) * 1609.344;
    QCOMPARE(kkFunction(speedMps), 164.435f);
    QCOMPARE(static_cast<int>(kkFunction(7.964800)), 201);
}

void VirtualPowerTest::testCycleopsFluid2()
{
    auto fluid2Function = virtualPowerFunctionForTrainer(VirtualPowerTrainer::CYCLEOPS_FLUID_2);
    float speedMph = 10.0f;
    float speedMps = (speedMph / 3600) * 1609.344;
    QCOMPARE(static_cast<int>(fluid2Function(speedMps)), 102);

    float speedKmph = 40.0;
    speedMps = speedKmph / 3.6f;
    QCOMPARE(static_cast<int>(fluid2Function(speedMps)), 408);
}



