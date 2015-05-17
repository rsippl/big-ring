#ifndef VIRTUALPOWERTEST_H
#define VIRTUALPOWERTEST_H

#include <QObject>

class VirtualPowerTest : public QObject
{
    Q_OBJECT
private slots:
    void testKurtKineticRoadMachine();
};

#endif // VIRTUALPOWERTEST_H
