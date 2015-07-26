#include "antmessage2test.h"
#include "virtualcyclingfileparsertest.h"
#include "virtualpowertest.h"
#include <QTest>
int main(int, char**) {
    AntMessage2Test antMessage2Test;
    QTest::qExec(&antMessage2Test);
    VirtualCyclingFileParserTest virtualCyclingFileParserTest;
    QTest::qExec(&virtualCyclingFileParserTest);
    VirtualPowerTest vpTest;
    QTest::qExec(&vpTest);
}
