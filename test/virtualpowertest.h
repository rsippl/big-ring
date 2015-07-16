#ifndef VIRTUALPOWERTEST_H
#define VIRTUALPOWERTEST_H

#include <QObject>

class VirtualPowerTest : public QObject
{
    Q_OBJECT
private slots:
    void testKurtKineticRoadMachine();
    void testCycleopsFluid2();
};

#endif // VIRTUALPOWERTEST_H
