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
    static const quint8 ANT_PLUS_NETWORK_NUMBER = 1;
    static const quint16 ANT_CHANNEL_FREQUENCY_BASE = 2400; // Mhz
    static const quint16 ANT_PLUS_CHANNEL_FREQUENCY = 2457; // Mhz

    /** message ids (in alphabetic order) */
    enum AntMessageId {
        ASSIGN_CHANNEL = 0x42,
        CHANNEL_EVENT = 0x40,
        SET_CHANNEL_FREQUENCY = 0x45,
        SET_CHANNEL_ID = 0x51,
        SET_CHANNEL_PERIOD = 0x43,
        SET_NETWORK_KEY = 0x46,
        SET_SEARCH_TIMEOUT = 0x44,
        SYSTEM_RESET = 0x4A,
        UNASSIGN_CHANNEL = 0x41
    };

    QByteArray toBytes() const;
    QByteArray toHex() const;
    virtual QString toString() const;

    AntMessageId id() const;

    // static factory methods for different messages
    static AntMessage2 assignChannel(quint8 channelNumber, quint8 channelType = 0, quint8 networkNumber = ANT_PLUS_NETWORK_NUMBER);
    static AntMessage2 setChannelFrequency(quint8 channelNumber, quint16 frequency = ANT_PLUS_CHANNEL_FREQUENCY);
    static AntMessage2 setChannelId(quint8 channelNumber, quint16 deviceId, quint8 deviceType);
    /**
     * @brief setChannelPeriod set the channel messaging period.
     * @param channelNumber the channel number
     * @param messageRate the message rate. This is the rate in seconds * 32768. To get a rate of 4Hz, set the channel
     * period to 8192. 32768 / 4 = 8192. This period is determined by the sender, so look at the device profile to
     * get the correct period. For the ANT+ Heart Rate Profile, this should be set to 8070, for a rate of 4.06Hz.
     * @return a ANT+ message for setting a channel's messaging period.
     */
    static AntMessage2 setChannelPeriod(quint8 channelNumber, quint16 messageRate);
    static AntMessage2 setNetworkKey(quint8 networkNumber, const std::array<quint8, 8> &networkKey);
    static AntMessage2 setSearchTimeout(quint8 channelNumber, int seconds);
    static AntMessage2 systemReset();
    static AntMessage2 unassignChannel(quint8 channelNumber);

protected:
    AntMessage2(const AntMessageId id, const QByteArray& content);
    const QByteArray& content() const;
    quint8 contentByte(int nr) const;
    quint16 contentShort(int index) const;

private:
    quint8 computeChecksum(const QByteArray& bytes) const;

    AntMessageId _id;

    QByteArray _content;
};

class AntChannelEventMessage: public AntMessage2
{
public:
    AntChannelEventMessage(const QByteArray& bytes);

    enum MessageCode {
        EVENT_CHANNEL_IN_WRONG_STATE = 0x15,
        EVENT_RESPONSE_NO_ERROR = 0x00
    };

    quint8 channelNumber() const;
    quint8 messageId() const;
    MessageCode messageCode() const;

    virtual QString toString() const override;
private:

    quint8 _channelNumber;
    quint8 _messageId;
    MessageCode _messageCode;
};

#endif // ANTMESSAGE2_H
