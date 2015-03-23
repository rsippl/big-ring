#include "antmessagetest.h"
#include <QtTest/QTest>

#define QCOMPARE_BYTE(actual, expected) \
do {\
    if (!QTest::qCompare(static_cast<const quint8>(actual), static_cast<uint8_t>(expected), #actual, #expected, __FILE__, __LINE__))\
        return;\
} while (0)

void AntMessageTest::systemReset()
{
    ANTMessage message = ANTMessage::resetSystem();
    QCOMPARE(message.messageType(), ANT_SYSTEM_RESET);
    QCOMPARE(message.payloadLength(), 1);
    QByteArray bytes = message.bytes();
    QCOMPARE(bytes.size(), 5);
    QCOMPARE_BYTE(bytes.at(0), AntSyncByte);
    QCOMPARE_BYTE(bytes.at(1), 1);
    QCOMPARE_BYTE(bytes.at(2), AntSystemReset);
    QCOMPARE_BYTE(bytes.at(3), 0);
    QCOMPARE_BYTE(bytes.at(4), 0xEF);

    QCOMPARE_BYTE(message.payLoad().at(0), 0u);
}

void AntMessageTest::setNetworkKey()
{
    const unsigned char networkKey[8] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
    ANTMessage msg = ANTMessage::setNetworkKey(2, networkKey);

    QCOMPARE_BYTE(msg.messageType(), AntSetNetworkKey);
    QByteArray bytes = msg.bytes();
    QByteArray payload = msg.payLoad();
    QByteArray key = msg.payLoad().mid(1);
    QCOMPARE(msg.payloadLength(), 9);
    QCOMPARE(key, QByteArray(reinterpret_cast<const char*>(networkKey), 8));
}





QTEST_MAIN(AntMessageTest)

