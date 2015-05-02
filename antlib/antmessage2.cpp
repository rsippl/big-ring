#include "antmessage2.h"
#include <QtCore/QtDebug>

namespace {
const double MESSAGING_PERIOD_BASE = 32768.0;

const QMap<AntChannelEventMessage::MessageCode,QString> EVENT_CHANNEL_MESSAGES (
{
            {AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR, "NO_ERROR"},
            {AntChannelEventMessage::EVENT_CHANNEL_IN_WRONG_STATE, "WRONG_STATE"},
            {AntChannelEventMessage::EVENT_CHANNEL_RX_FAIL, "RX_FAIL"},
            {AntChannelEventMessage::EVENT_CHANNEL_RX_SEARCH_TIMEOUT, "SEARCH_TIMEOUT"}
});
}


AntMessage2::AntMessage2(const QByteArray &completeMessageBytes):
    AntMessage2(static_cast<AntMessageId>(completeMessageBytes[2]),
    completeMessageBytes.mid(3, completeMessageBytes[1]))
{
    // empty
}

AntMessage2::AntMessage2(const AntMessageId id, const QByteArray& content):
    _id(id), _content(content)
{
    // empty
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

AntMessage2::AntMessage2():
    AntMessage2(INVALID, QByteArray())
{

}

bool AntMessage2::isNull() const
{
    return _id == INVALID;
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
    case OPEN_CHANNEL:
        return QString("Open Channel (0x4b), Channel %1").arg(contentByte(0));
    case REQUEST_MESSAGE:
        return QString("Request Message, Channel %1, Message Id %2").arg(contentByte(0)).arg(QString::number(contentByte(1), 16));
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
        return QString("Unknown message %1").arg(QString::number(_id, 16));
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

const AntChannelEventMessage* AntMessage2::asChannelEventMessage() const
{
    const AntChannelEventMessage* antChannelEventMessage = dynamic_cast<const AntChannelEventMessage*>(this);
    Q_ASSERT_X(antChannelEventMessage, "AntMessage2::asChannelEventMessage()", "bad cast to ChannelEventMessage");
    return antChannelEventMessage;
}

AntMessage2 AntMessage2::assignChannel(quint8 channelNumber, quint8 channelType, quint8 networkNumber)
{
    QByteArray data;
    data += channelNumber;
    data += channelType;
    data += networkNumber;

    return AntMessage2(ASSIGN_CHANNEL, data);
}

AntMessage2 AntMessage2::openChannel(quint8 channelNumber)
{
    QByteArray content;
    content += channelNumber;

    return AntMessage2(OPEN_CHANNEL, content);
}

AntMessage2 AntMessage2::requestMessage(quint8 channelNumber, AntMessage2::AntMessageId messageId)
{
    QByteArray content;
    content += channelNumber;
    content += static_cast<quint8>(messageId);

    return AntMessage2(REQUEST_MESSAGE, content);
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
    QString channelEventString = EVENT_CHANNEL_MESSAGES.value(_messageCode, "UNKNOWN");
    return QString("Channel Event %1, Channel %2, Message 0x%3").arg(channelEventString)
            .arg(contentByte(0)).arg(QString::number(_messageId, 16));
}


std::unique_ptr<AntMessage2> AntMessage2::createMessageFromBytes(const QByteArray &bytes)
{
    switch(bytes[2]) {
    case AntMessage2::CHANNEL_EVENT:
        return std::unique_ptr<AntMessage2>(new AntChannelEventMessage(bytes));
    default:
        return std::unique_ptr<AntMessage2>(new AntMessage2(bytes));
    }

    return std::unique_ptr<AntMessage2>();
}

HeartRateMessage::HeartRateMessage(const AntMessage2 &antMessage): BroadCastMessage(antMessage)
{
    _measurementTime = antMessage.contentShort(5);
    _heartBeatCount = antMessage.contentByte(7);
    _computedHeartRate = antMessage.contentByte(8);
}

quint16 HeartRateMessage::measurementTime() const
{
    return _measurementTime;
}

quint8 HeartRateMessage::heartBeatCount() const
{
    return _heartBeatCount;
}

quint8 HeartRateMessage::computedHeartRate() const
{
    return _computedHeartRate;
}



BroadCastMessage::BroadCastMessage(const AntMessage2 &antMessage):
    _antMessage(antMessage),
    _channelNumber(antMessage.contentByte(0)),
    _dataPage(antMessage.contentByte(1))
{
    // empty
}

quint8 BroadCastMessage::channelNumber() const
{
    return _channelNumber;
}

quint8 BroadCastMessage::dataPage() const
{
    return _dataPage;
}

const AntMessage2 &BroadCastMessage::antMessage() const
{
    return _antMessage;
}

PowerMessage::PowerMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{

}

bool PowerMessage::isPowerOnlyPage() const
{
    return dataPage() == POWER_ONLY_PAGE;
}

quint8 PowerMessage::eventCount() const
{
    return antMessage().contentByte(2);
}

quint8 PowerMessage::instantaneousCadence() const
{
    return antMessage().contentByte(4);
}

quint16 PowerMessage::accumulatedPower() const
{
    return antMessage().contentShort(5);
}

quint16 PowerMessage::instantaneousPower() const
{
    return antMessage().contentShort(7);
}

SpeedAndCadenceMessage::SpeedAndCadenceMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 SpeedAndCadenceMessage::cadenceEventTime() const
{
    return antMessage().contentShort(1);
}

quint16 SpeedAndCadenceMessage::pedalRevolutions() const
{
    return antMessage().contentShort(3);
}

quint16 SpeedAndCadenceMessage::speedEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 SpeedAndCadenceMessage::wheelRevolutions() const
{
    return antMessage().contentShort(7);
}


SpeedMessage::SpeedMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 SpeedMessage::speedEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 SpeedMessage::wheelRevolutions() const
{
    return antMessage().contentShort(7);
}


CadenceMessage::CadenceMessage(const AntMessage2 &antMessage):
    BroadCastMessage(antMessage)
{
    // empty
}

quint16 CadenceMessage::cadenceEventTime() const
{
    return antMessage().contentShort(5);
}

quint16 CadenceMessage::pedalRevolutions() const
{
    return antMessage().contentShort(7);
}
