#include "antmessage2test.h"
#include "virtualpowertest.h"
#include <QTest>
int main(int, char**) {
    AntMessage2Test antMessage2Test;
    QTest::qExec(&antMessage2Test);
    VirtualPowerTest vpTest;
    QTest::qExec(&vpTest);
}
