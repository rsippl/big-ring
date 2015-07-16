/*
 * Copyright (c) 2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "virtualpower.h"
#include <QtCore/QtDebug>
#include <QtCore/QtMath>
namespace
{
const float MILES_PER_METERS = 0.000621371192;
const float METERS_PER_SECOND_TO_MILES_PER_HOUR = MILES_PER_METERS * 3600;

float metersPerSecondToMilesPerHour(float metersPerSecond) {
    return metersPerSecond * METERS_PER_SECOND_TO_MILES_PER_HOUR;
}

float kurtKineticRoadMachine(float wheelSpeedMps) {
    float wheelSpeedMilesPerHour = metersPerSecondToMilesPerHour(wheelSpeedMps);
    return 5.244820 * wheelSpeedMilesPerHour +
            0.019168 * qPow(wheelSpeedMilesPerHour, 3);
}

float cycleopsFluid2(float wheelSpeedMps) {
    float wheelSpeedMilesPerHour = metersPerSecondToMilesPerHour(wheelSpeedMps);

    return 0.0115 * qPow(wheelSpeedMilesPerHour, 3) +
            0.0137 * qPow(wheelSpeedMilesPerHour, 2) +
            8.9788 * wheelSpeedMilesPerHour;
}
using indoorcycling::VirtualPowerTrainer;

const QMap<VirtualPowerTrainer,std::function<float(float)>> VIRTUAL_POWER_FUNCTIONS = {
{VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE, kurtKineticRoadMachine},
{VirtualPowerTrainer::CYCLEOPS_FLUID_2, cycleopsFluid2},
};
}
namespace indoorcycling
{

VirtualPowerFunctionType virtualPowerFunctionForTrainer(VirtualPowerTrainer trainer)
{
    auto it = VIRTUAL_POWER_FUNCTIONS.find(trainer);
    if (it == VIRTUAL_POWER_FUNCTIONS.end()) {
        // return the identity function if nothing's matched.
        return [](float f) { return f; };
    } else {
        return *it;
    }
}

}
