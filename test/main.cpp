#include "antmessage2test.h"
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
}
