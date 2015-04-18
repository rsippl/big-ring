#include "antmessage2test.h"
#include "antmessage2.h"
#include "common.h"

#include <QtCore/QtDebug>
#include <QtCore/QtEndian>

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

void AntMessage2Test::unassignChannel()
{
    AntMessage2 msg = AntMessage2::unassignChannel(3u);
    QCOMPARE_BYTE(msg.id(), AntMessage2::UNASSIGN_CHANNEL);
    QCOMPARE(msg.toBytes().size(), 5);
    int channel = msg.toBytes()[3];
    QCOMPARE(channel, 3);
}

void AntMessage2Test::assignChannel()
{
    AntMessage2 msg = AntMessage2::assignChannel(1, 0x10, 3);
    QCOMPARE_BYTE(msg.id(), AntMessage2::ASSIGN_CHANNEL);
    QCOMPARE(msg.toBytes().size(), 7);
    QByteArray content = msg.toBytes().mid(3, 3);
    int channel = content[0];
    QCOMPARE(channel, 1);
    int channelType = content[1];
    QCOMPARE(channelType, 0x10);
    int networkNumber = content[2];
    QCOMPARE(networkNumber, 3);
}

void AntMessage2Test::setChannelId()
{
    quint16 theDeviceId = 10000;
    int theDeviceType = 3;
    AntMessage2 msg = AntMessage2::setChannelId(1, theDeviceId, theDeviceType);

    QCOMPARE_BYTE(msg.id(), AntMessage2::SET_CHANNEL_ID);
    QCOMPARE(msg.toBytes().size(), 9);
    QByteArray content = msg.toBytes().mid(3, 5);
    int channel = content[0];
    QCOMPARE(channel, 1);
    const uchar* deviceIdChars = reinterpret_cast<uchar*>(&content.data()[1]);
    quint16 deviceId = qFromLittleEndian<quint16>(deviceIdChars);
    QCOMPARE(deviceId, deviceId);
    int deviceType = content[3];
    QCOMPARE(deviceType, theDeviceType);
    int transmissionType = content[4];
    QCOMPARE(transmissionType, 0);
}

void AntMessage2Test::channelEventNoError()
{
    QByteArray bytes = QByteArray::fromHex(QByteArray::fromRawData("a40340014615a0", 14));
    AntChannelEventMessage msg(bytes);

    QCOMPARE(msg.id(), AntMessage2::CHANNEL_EVENT);
    quint8 channelNumber = 1;
    QCOMPARE(msg.channelNumber(), channelNumber);
    int messageId = msg.messageId();
    QCOMPARE(messageId, 0x46);
    int actualMessageCode = msg.messageCode();
    int expectedMessageCode = AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR;
    QCOMPARE(actualMessageCode, expectedMessageCode);
}
