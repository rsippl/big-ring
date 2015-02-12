#include "quantityprinter.h"
#include <QtCore/QtDebug>
namespace {
const double METERS_PER_MILE = 1609.344;
const double MILES_PER_METER = 1.0 / METERS_PER_MILE;
// meters per second to miles per hour.
const double MPH_PER_MPS = MILES_PER_METER * 3600;

const double YARDS_PER_METER = 1.0936;
}
QuantityPrinter::QuantityPrinter(QObject *parent) :
    QObject(parent)
{
    // empty
}

QString QuantityPrinter::unitString(QuantityPrinter::Quantity quantity, QuantityPrinter::Precision precision, QVariant value) const
{
    switch(quantity) {
    case Distance:
        return unitForDistance(precision, value);
    case Speed:
        return (system() == ImperialSystem) ? "MPH" : "KM/H";
    case Cadence:
        return tr("RPM");
    case Power:
        return tr("W");
    case HeartRate:
        return tr("BPM");
    case Grade:
        return "%";
    default:
        return "#";
    }
}

QString QuantityPrinter::unitForDistance(QuantityPrinter::Precision precision, QVariant value) const
{
    if (system() == ImperialSystem) {
        if (precision == Precise && value.toReal() < METERS_PER_MILE) {
            return "Y";
        }
        return "Mi";
    }
    if (precision == Precise && value.toReal() < 1000) {
        return "M";
    }
    return "KM";
}

QString QuantityPrinter::print(QVariant value, QuantityPrinter::Quantity quantity, Precision precision, int width) const
{
    switch(quantity) {
    case Distance:
        return printDistance(value.toReal(), precision, width);
    case Speed:
        return printSpeed(value.toReal(), width);
    case Cadence:
        return QString("%1").arg(value.toInt(), width);
    case Power:
        return QString("%1").arg(value.toInt(), width);
    case HeartRate:
        return QString("%1").arg(value.toInt(), width);
    case Grade:
        return QString("%1").arg(value.toReal(), width, 'f', 1);
    default:
        return "Unknown";
    }
}

QString QuantityPrinter::printDistance(qreal meters, Precision precision, int width) const
{
    if (system() == ImperialSystem) {
        if (precision == Precise && meters < METERS_PER_MILE) {
            return QString("%1").arg(meters * YARDS_PER_METER, width, 'f', 0);
        }
        return QString("%1").arg(meters * MILES_PER_METER, width, 'f', 2);
    }
    if (precision == Precise && meters < 1000) {
        return QString("%1").arg(meters, width, 'f', 0);
    }
    return QString("%1").arg(meters / 1000, width, 'f', 2);
}

QString QuantityPrinter::printSpeed(qreal metersPerSecond, int width) const
{
    if (system() == ImperialSystem) {
        return QString("%1").arg(metersPerSecond * MPH_PER_MPS, width, 'f', 1);
    }
    return QString("%1").arg(metersPerSecond * 3.6,  width, 'f', 1);
}

QuantityPrinter::System QuantityPrinter::system() const
{
    QString systemString = _settings.value("units").toString();
    if (systemString == "Imperial") {
        return ImperialSystem;
    }
    return MetricSystem;
}
