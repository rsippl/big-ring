#include "antmessage2test.h"
#include "antmessage2.h"
#include "common.h"

#include <QtCore/QtDebug>

void AntMessage2Test::systemReset()
{
    AntMessage2 resetMessage = AntMessage2::systemReset();

    QCOMPARE_BYTE(resetMessage.id(), AntMessage2::SYSTEM_RESET);
    QByteArray bytes = resetMessage.toBytes();

    qDebug() << bytes.size();
    QCOMPARE(bytes.size(), 5);
    QCOMPARE_BYTE(bytes[0], AntMessage2::SYNC_BYTE);
    QCOMPARE_BYTE(bytes[1], 1);

    qDebug() << bytes.toHex();
}

void AntMessage2Test::setNetworkKey()
{
    std::array<quint8,8> theKey = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8 };
    AntMessage2 msg = AntMessage2::setNetworkKey(4, theKey);

    QCOMPARE_BYTE(msg.id(), AntMessage2::SET_NETWORK_KEY);
    QByteArray bytes = msg.toBytes();

    QCOMPARE(bytes.size(), 13);
    QCOMPARE_BYTE(bytes[0], AntMessage2::SYNC_BYTE);
    QCOMPARE_BYTE(bytes[1], 9);
    QByteArray key = bytes.mid(4, 8);
    QByteArray comparison = QByteArray::fromRawData(reinterpret_cast<char*>(&theKey), 8);
    QCOMPARE(key, comparison);
}
