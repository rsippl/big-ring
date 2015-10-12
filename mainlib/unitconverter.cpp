#include "unitconverter.h"

namespace {
const double METERS_PER_KILOMETER = 1000;
const double METERS_PER_MILE = 1609.344;
const double MILES_PER_METER = 1.0 / METERS_PER_MILE;
const double YARDS_PER_METER = 1.0936;
const double FEET_PER_METER = 3.2808399;
// meters per second to miles per hour.
const double MPH_PER_MPS = MILES_PER_METER * 3600;
const double KMPH_PER_MPS = 3600 / METERS_PER_KILOMETER;
}

UnitConverter::UnitConverter(QObject *parent): QObject(parent)
{
    // empty
}

UnitConverter::System UnitConverter::system()
{
    QString systemString = QSettings().value("units").toString();
    if (systemString == "Imperial") {
        return System::Imperial;
    }
    return System::Metric;
}

UnitConverter::DistanceUnit UnitConverter::distanceUnitForSystem()
{
    switch(system()) {
    case System::Imperial:
        return DistanceUnit::Mile;
    default:
        return DistanceUnit::Kilometer;
    }
}

UnitConverter::DistanceUnit UnitConverter::altitudeUnitForSystem()
{
    switch(system()) {
    case System::Imperial:
        return DistanceUnit::Foot;
    default:
        return DistanceUnit::Meter;
    }
}

UnitConverter::SpeedUnit UnitConverter::speedUnitForSystem()
{
    switch(system()) {
    case System::Imperial:
        return SpeedUnit::MilesPerHour;
    default:
        return SpeedUnit::KilometersPerHour;
    }
}

double UnitConverter::convertDistanceTo(double distance, UnitConverter::DistanceUnit unit)
{
    switch(unit) {
    case DistanceUnit::Kilometer:
        return distance / 1000.0;
    case DistanceUnit::Mile:
        return distance / METERS_PER_MILE;
    case DistanceUnit::Yard:
        return distance * YARDS_PER_METER;
    case DistanceUnit::Foot:
        return distance * FEET_PER_METER;
    default:
        return distance;
    }
}

double UnitConverter::convertDistanceFrom(double distance, UnitConverter::DistanceUnit unit)
{
    switch(unit) {
    case DistanceUnit::Kilometer:
        return distance * 1000.0;
    case DistanceUnit::Mile:
        return distance * METERS_PER_MILE;
    case DistanceUnit::Yard:
        return distance / YARDS_PER_METER;
    case DistanceUnit::Foot:
        return distance / FEET_PER_METER;
    default:
        return distance;
    }
}

double UnitConverter::convertSpeedTo(double speedMetersPerSecond, UnitConverter::SpeedUnit unit)
{
    switch (unit) {
    case SpeedUnit::KilometersPerHour:
        return speedMetersPerSecond * KMPH_PER_MPS;
    case SpeedUnit::MilesPerHour:
        return speedMetersPerSecond * MPH_PER_MPS;
    default:
        return speedMetersPerSecond;
    }
}

double UnitConverter::convertAltitudeTo(double meters, UnitConverter::DistanceUnit unit)
{
    switch(unit) {
    case DistanceUnit::Foot:
        return meters * FEET_PER_METER;
    default:
        return meters;
    }
}
