#include "antmessage2.h"
#include <QtCore/QtDebug>
#include <QtCore/QtEndian>

AntMessage2::AntMessage2(const AntMessageId id, const QByteArray& content):
    _id(id), _content(content)
{
}

quint8 AntMessage2::computeChecksum(const QByteArray &bytes) const
{
    quint8 checksum = 0;
    for (int i = 0; i < bytes.length(); ++i) {
        checksum ^= bytes[i];
    }
    return checksum;
}

quint8 AntMessage2::contentByte(int nr) const
{
    return static_cast<quint8>(_content[nr]);
}

quint8 AntMessage2::contentShort(int index) const
{
    const uchar* deviceIdChars = reinterpret_cast<const uchar*>(&_content.data()[index]);
    return qFromLittleEndian<quint16>(deviceIdChars);
}

QByteArray AntMessage2::toBytes() const
{
    QByteArray bytes;
    bytes.append(SYNC_BYTE);
    bytes.append(static_cast<quint8>(_content.length()));
    bytes.append(static_cast<quint8>(_id));
    bytes.append(_content);
    bytes.append(computeChecksum(bytes));

    return bytes;
}

QByteArray AntMessage2::toHex() const
{
    return toBytes().toHex();
}

QString AntMessage2::toString() const
{
    switch(_id) {
    case ASSIGN_CHANNEL:
        return QString("Assign Channel, Channel %1, Channel Type %2, Network Number %3").arg(contentByte(0)).arg(contentByte(1)).arg(contentByte(2));
    case SET_CHANNEL_ID:
        return QString("Set Channel Id, Channel %1, Device Nr %2, "
                       "Device Type %3").arg(contentByte(0)).arg(contentShort(1)).arg(contentByte(3));
    case SET_NETWORK_KEY:
        return QString("Set Network Key, Network %1, Key: %2").arg(contentByte(0)).arg(QString(_content.mid(1).toHex()));
    case SYSTEM_RESET:
        return "Sytem Reset";
    case UNASSIGN_CHANNEL:
        return QString("Unassign Channel, Channel #%1").arg(contentByte(0));
    default:
        return "Unknown message";
    }
}

AntMessage2::AntMessageId AntMessage2::id() const
{
    return _id;
}

AntMessage2 AntMessage2::systemReset()
{
    static const char data[] = { 0x00, 0x00 };
    return AntMessage2(SYSTEM_RESET, QByteArray::fromRawData(data, 1));
}

AntMessage2 AntMessage2::setNetworkKey(quint8 networkNumber, const std::array<quint8, 8>& networkKey)
{
    QByteArray content;
    content += networkNumber;
    for(const quint8 byte: networkKey) {
        content += byte;
    }
    return AntMessage2(SET_NETWORK_KEY, content);
}

AntMessage2 AntMessage2::unassignChannel(quint8 channelNumber)
{
    QByteArray array;
    array += channelNumber;

    return AntMessage2(UNASSIGN_CHANNEL, array);
}

AntMessage2 AntMessage2::assignChannel(quint8 channelNumber, quint8 channelType, quint8 networkNumber)
{
    QByteArray data;
    data += channelNumber;
    data += channelType;
    data += networkNumber;

    return AntMessage2(ASSIGN_CHANNEL, data);
}

AntMessage2 AntMessage2::setChannelId(quint8 channelNumber, quint16 deviceId, quint8 deviceType)
{
    QByteArray array;
    array += channelNumber;
    array += deviceId & 0xFF;
    array += (deviceId >> 8) & 0xFF;
    array += deviceType;
    quint8 zero = 0u;
    array += zero;

    return AntMessage2(SET_CHANNEL_ID, array);
}
