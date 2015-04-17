#include "antmessage2test.h"
#include "antmessagetest.h"

#include <QTest>
int main(int argc, char** argv) {
    AntMessageTest antMessageTest;
    AntMessage2Test antMessage2Test;

    QTest::qExec(&antMessageTest);
    QTest::qExec(&antMessage2Test);
}
