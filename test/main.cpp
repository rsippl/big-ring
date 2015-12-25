#include "antmessage2test.h"
#include "distanceentrycollectiontest.h"
#include "profiletest.h"
#include "reallifevideocachetest.h"
#include "ridefilewritertest.h"
#include "rollingaveragecalculatortest.h"
#include "virtualtrainingfileparsertest.h"
#include "virtualpowertest.h"

#include <QTest>

int main(int, char**) {
    AntMessage2Test antMessage2Test;
    QTest::qExec(&antMessage2Test);

    VirtualTrainingFileParserTest virtualTrainingFileParserTest;
    QTest::qExec(&virtualTrainingFileParserTest);
    VirtualPowerTest vpTest;
    QTest::qExec(&vpTest);
    ProfileTest profileTest;
    QTest::qExec(&profileTest);
    RollingAverageCalculatorTest racTest;
    QTest::qExec(&racTest);
    RideFileWriterTest rideFileWriterTest;
    QTest::qExec(&rideFileWriterTest);
    RealLifeVideoCacheTest saveAndLoadTest;
    QTest::qExec(&saveAndLoadTest);

    DistanceEntryCollectionTest distanceEntryCollectionTest;
    QTest::qExec(&distanceEntryCollectionTest);
}
