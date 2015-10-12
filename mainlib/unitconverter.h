#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class UnitConverter : public QObject
{
    Q_OBJECT
public:
    /** The Unit Systems that can be converted to */
    enum class System {
        Metric, Imperial
    };

    enum class DistanceUnit {
        Meter,
        Kilometer,
        Yard,
        Mile,
        Foot
    };
    enum class SpeedUnit {
        MetersPerSecond,
        KilometersPerHour,
        MilesPerHour
    };

    explicit UnitConverter(QObject *parent);

    static System system();
    static DistanceUnit distanceUnitForSystem();
    static DistanceUnit altitudeUnitForSystem();
    static SpeedUnit speedUnitForSystem();
    /** convert a distance to the distance unit provided */
    double convertDistanceTo(double distance, DistanceUnit unit = distanceUnitForSystem());
    /** convert a distance from the distance unit provided, returning the distance in SI */
    double convertDistanceFrom(double distance, DistanceUnit unit = distanceUnitForSystem());

    double convertSpeedTo(double speedMetersPerSecond, SpeedUnit unit = speedUnitForSystem());

    double convertAltitudeTo(double meters, DistanceUnit unit = altitudeUnitForSystem());

private:
    QSettings _settings;
};

#endif // UNITCONVERTER_H
