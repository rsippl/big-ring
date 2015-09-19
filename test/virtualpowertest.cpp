#include "virtualpowertest.h"

#include "common.h"
#include "mainlib/virtualpower.h"

#include <memory>
namespace {
float mphToMps(float speedMph) {
    return (speedMph / 3600) * 1609.344;
}
float kmphToMps(float kmph) {
    return kmph / 3.6;
}

}
using namespace indoorcycling;
void VirtualPowerTest::testKurtKineticRoadMachine()
{
    VirtualPowerFunctionType kkFunction = virtualPowerFunctionForTrainer(
                VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE);

    QCOMPARE(kkFunction(mphToMps(16.1f)), 164.435f);
    QCOMPARE(static_cast<int>(kkFunction(7.964800)), 201);
}

void VirtualPowerTest::testCycleopsFluid2()
{
    auto fluid2Function = virtualPowerFunctionForTrainer(VirtualPowerTrainer::CYCLEOPS_FLUID_2);
    QCOMPARE(static_cast<int>(fluid2Function(mphToMps(10))), 102);

    QCOMPARE(static_cast<int>(fluid2Function(kmphToMps(40.0))), 408);
}

void VirtualPowerTest::testEliteQuboFluid()
{
    auto eliteQuboFluidFunction = virtualPowerFunctionForTrainer(VirtualPowerTrainer::ELITE_QUBO_POWER_FLUID);
    float speedKmph = 25.0f;
    float speedMps = speedKmph / 3.6;
    QCOMPARE(static_cast<int>(eliteQuboFluidFunction(speedMps)), 238);
    QCOMPARE(static_cast<int>(eliteQuboFluidFunction(kmphToMps(41.5))), 807);
}



