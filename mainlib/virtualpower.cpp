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
using indoorcycling::VirtualPowerTrainer;
using indoorcycling::VirtualPowerFunctionType;

const float MILES_PER_METERS = 0.000621371192;
const float METERS_PER_SECOND_TO_MILES_PER_HOUR = MILES_PER_METERS * 3600;
const float METERS_PER_SECOND_TO_KILOMETERS_PER_HOUR = 3.6;

enum class SpeedUnit {
    METERS_PER_SECOND,
    KILOMETERS_PER_HOUR,
    MILES_PER_HOUR
};

struct PowerCoefficients {
    SpeedUnit speedUnit;
    float first;
    float second;
    float third;
};

const PowerCoefficients IDENTITY_POWER_CURVE = { SpeedUnit::METERS_PER_SECOND, 1, 0, 0 };

const std::map<VirtualPowerTrainer, PowerCoefficients> powerCurves = {
    // Kurt Kinetic Road Machine: P = 5.244820 * MPH + 0.019168 * MPH^3
    { VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE, { SpeedUnit::MILES_PER_HOUR, 5.244820, 0, 0.019168 }},
    // Kurt Kinetic Cyclone: P = 6.481090 * mph + 0.020106 * (mph*mph*mph)
    { VirtualPowerTrainer::KURT_KINETIC_ROAD_MACHINE, { SpeedUnit::MILES_PER_HOUR, 6.481090, 0, 0.020106 }},
    // Cycleops Fluid 2: P = 8.9788 * MPH + 0.0137 * MPH^2 + 0.0115 * MPH^3
    { VirtualPowerTrainer::CYCLEOPS_FLUID_2, { SpeedUnit::MILES_PER_HOUR, 8.9788, 0.0137, 0.0115 }},
    // ELITE QUBO POWER FLUID f(x) = 4.31746 * kmph -2.59259e-002 * kmph^2 +  9.41799e-003 * kmph^3
    { VirtualPowerTrainer::ELITE_QUBO_POWER_FLUID, { SpeedUnit::KILOMETERS_PER_HOUR, 4.31746, -2.59259e-002, 9.41799e-003 }},
    // Elite Turbo Muin 2013 model P =  0.00791667 v^3 + 0.125 v^2 - 0.16669 v
    { VirtualPowerTrainer::ELITE_TURBO_MUIN_2013, { SpeedUnit::KILOMETERS_PER_HOUR, - 0.16669, 0.125, 0.00791667 }}
};
/**
 * Convert meters per second to miles per hour
 * @param metersPerSecond speed in meters per second
 * @return speed in miles per hour
 */
float metersPerSecondToMilesPerHour(float metersPerSecond) {
    return metersPerSecond * METERS_PER_SECOND_TO_MILES_PER_HOUR;
}

/**
 * Convert meters per second to kilometers per hour
 * @param metersPerSecond speed in meters per second
 * @return speed in miles per hour
 */
float metersPerSecondToKilometersPerHour(float metersPerSecond) {
    return metersPerSecond * METERS_PER_SECOND_TO_KILOMETERS_PER_HOUR;
}

const PowerCoefficients &findPowerCurve(VirtualPowerTrainer trainer)
{
    auto it = powerCurves.find(trainer);
    if (it == powerCurves.end()) {
        return IDENTITY_POWER_CURVE;
    }
    return (*it).second;
}

const std::function<float(float)> determineSpeedConversionFunction(VirtualPowerTrainer trainer) {
    switch (findPowerCurve(trainer).speedUnit) {
    case SpeedUnit::KILOMETERS_PER_HOUR:
        return &metersPerSecondToKilometersPerHour;
    case SpeedUnit::MILES_PER_HOUR:
        return &metersPerSecondToMilesPerHour;
    default:
        return [](float mps) { return mps; };
    }
}
}

namespace indoorcycling
{

VirtualPowerFunctionType virtualPowerFunctionForTrainer(VirtualPowerTrainer trainer)
{
    const PowerCoefficients &coefficients = findPowerCurve(trainer);
    std::function<float(float)> speedConversionFunction = determineSpeedConversionFunction(trainer);

    return [coefficients, speedConversionFunction](float speedMetersPerSecond) {
        float convertedSpeed = speedConversionFunction(speedMetersPerSecond);
        return coefficients.first * convertedSpeed +
                coefficients.second * qPow(convertedSpeed, 2) +
                coefficients.third * qPow(convertedSpeed, 3);
    };
}
}
