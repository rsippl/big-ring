#ifndef QUANTITYPRINTER_H
#define QUANTITYPRINTER_H

#include <QtCore/QObject>
#include <QtCore/QSettings>

class QuantityPrinter : public QObject
{
    Q_OBJECT
public:
    enum Precision {
        NonPrecise,
        Precise
    };

    enum Quantity {
        Distance,
        Speed,
        Power,
        HeartRate,
        Cadence,
        Grade
    };

    enum System {
        MetricSystem, ImperialSystem
    };

    explicit QuantityPrinter(QObject *parent = 0);

    QString unitString(Quantity quantity, QuantityPrinter::Precision precision = NonPrecise, QVariant value = QVariant::fromValue(0.0)) const;
    QString unitForDistance(QuantityPrinter::Precision precision, QVariant value) const;
    QString print(QVariant value, Quantity quantity, Precision = NonPrecise, int width = 5) const;
    QString printDistance(qreal meters, Precision = NonPrecise, int width = 5) const;
    QString printSpeed(qreal metersPerSecond, int width) const;
private:
    System system() const;

    QSettings _settings;

};

#endif // QUANTITYPRINTER_H
