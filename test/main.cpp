#include "antmessage2test.h"
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
    RollingAverageCalculatorTest racTest;
    QTest::qExec(&racTest);
}
