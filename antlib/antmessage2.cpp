#include "antmessage2.h"
#include <QtCore/QtDebug>

namespace {
const double MESSAGING_PERIOD_BASE = 32768.0;
}

AntMessage2::AntMessage2(const AntMessageId id, const QByteArray& content):
    _id(id), _content(content)
{
}

const QByteArray &AntMessage2::content() const
{
    return _content;
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

quint16 AntMessage2::contentShort(int index) const
{
    quint8 lowByte = _content[index];
    quint8 highByte = _content[index + 1];

    return ((highByte << 8) & 0xFF00) + lowByte;
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
    case SET_CHANNEL_FREQUENCY:
        return QString("Set Channel Frequency, Channel %1, Frequency %2Mhz").arg(contentByte(0)).arg(contentByte(1) + ANT_CHANNEL_FREQUENCY_BASE);
    case SET_CHANNEL_ID:
        return QString("Set Channel Id, Channel %1, Device Nr %2, "
                       "Device Type %3").arg(contentByte(0)).arg(contentShort(1)).arg(contentByte(3));
    case SET_CHANNEL_PERIOD:
        return QString("Set Channel Period, Channel %1, Period %2Hz (%3)").arg(contentByte(0))
                .arg(QString::number(MESSAGING_PERIOD_BASE / contentShort(1), 'f', 2)).arg(contentShort(1));
    case SET_NETWORK_KEY:
        return QString("Set Network Key, Network %1, Key: %2").arg(contentByte(0)).arg(QString(_content.mid(1).toHex()));
    case SET_SEARCH_TIMEOUT:
        return QString("Set Search Timeout, Channel %1, Timeout %2s").arg(contentByte(0)).arg(qRound(contentByte(1) * 2.5));
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

AntMessage2 AntMessage2::setSearchTimeout(quint8 channelNumber, int seconds)
{
    QByteArray content;
    content += channelNumber;
    quint8 timeout = static_cast<quint8>(qRound(seconds / 2.5));
    content += timeout;
    return AntMessage2(SET_SEARCH_TIMEOUT, content);
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

AntMessage2 AntMessage2::setChannelFrequency(quint8 channelNumber, quint16 frequency)
{
    QByteArray data;
    data += channelNumber;
    quint8 frequencyOffset = static_cast<quint8>(frequency - ANT_CHANNEL_FREQUENCY_BASE);
    data += frequencyOffset;
    return AntMessage2(SET_CHANNEL_FREQUENCY, data);
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

AntMessage2 AntMessage2::setChannelPeriod(quint8 channelNumber, quint16 messageRate)
{
    QByteArray content;
    content += channelNumber;
    content += messageRate & 0xFF;
    content += (messageRate >> 8) & 0xFF;

    return AntMessage2(SET_CHANNEL_PERIOD, content);
}


AntChannelEventMessage::AntChannelEventMessage(const QByteArray &bytes):
    AntMessage2(CHANNEL_EVENT, bytes.mid(3))
{
    _channelNumber = content()[0];
    _messageId = content()[1];
    quint8 messageCode = content()[2];
    _messageCode = static_cast<MessageCode>(messageCode);
}

quint8 AntChannelEventMessage::channelNumber() const
{
    return _channelNumber;
}

quint8 AntChannelEventMessage::messageId() const
{
    return _messageId;
}

AntChannelEventMessage::MessageCode AntChannelEventMessage::messageCode() const
{
    return _messageCode;
}

QString AntChannelEventMessage::toString() const
{
    switch (_messageCode) {
    case EVENT_CHANNEL_IN_WRONG_STATE:
        return QString("Channel Event CHANNEL_IN_WRONG_STATE, Channel #%1, Message 0x%2").arg(contentByte(0))
                .arg(QString::number(_messageId, 16));
    case EVENT_RESPONSE_NO_ERROR:
        return QString("Channel Event RESPONSE_NO_ERROR, Channel #%1, Message 0x%2").arg(contentByte(0))
                 .arg(QString::number(_messageId, 16));
    default:
        return "Channel Event Unknown";
    }
}
