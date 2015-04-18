#ifndef ANTMESSAGE2_H
#define ANTMESSAGE2_H

#include <array>
#include <QObject>

/**
 * Class representing AntMessages.
 * @brief The AntMessage2 class
 */
class AntMessage2
{
public:
    static const quint8 SYNC_BYTE = 0xA4;

    /** message ids (in alphabetic order) */
    enum AntMessageId {
        ASSIGN_CHANNEL = 0x42,
        SET_NETWORK_KEY = 0x46,
        SYSTEM_RESET = 0x4A,
        UNASSIGN_CHANNEL = 0x41
    };

    QByteArray toBytes() const;
    QByteArray toHex() const;
    QString toString() const;

    AntMessageId id() const;

    // static factory methods for different messages
    static AntMessage2 systemReset();
    static AntMessage2 setNetworkKey(quint8 networkNumber, const std::array<quint8, 8> &networkKey);
    static AntMessage2 unassignChannel(quint8 channelNumber);
    static AntMessage2 assignChannel(quint8 channelNumber, quint8 channelType, quint8 networkNumber);
private:
    AntMessage2(const AntMessageId id, const QByteArray& content);

    quint8 computeChecksum(const QByteArray& bytes) const;
    quint8 byte(int nr) const;

    AntMessageId _id;

    QByteArray _content;
};

class AntChannelEventMessage: public AntMessage2
{
public:
    AntChannelEventMessage(const QByteArray& bytes);

    enum MessageCode {
        EVENT_RESPONSE_NO_ERROR = 0x00
    };

    quint8 channel() const;
    quint8 messageId() const;
    MessageCode messageCode() const;


private:

    quint8 _channel;
    quint8 _messageId;
    MessageCode _messageCode;
};


#endif // ANTMESSAGE2_H
