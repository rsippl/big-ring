#ifndef ANTMESSAGE2_H
#define ANTMESSAGE2_H

#include <array>
#include <memory>
#include <QtCore/QObject>

/* forward declarations */
class AntChannelEventMessage;

/**
 * Class representing AntMessages.
 * @brief The AntMessage2 class
 */
class AntMessage2
{
public:
    /** creates an invalid AntMessage */
    AntMessage2();
    static const quint8 SYNC_BYTE = 0xA4;
    static const quint8 ANT_PLUS_NETWORK_NUMBER = 1;
    static const quint16 ANT_CHANNEL_FREQUENCY_BASE = 2400; // Mhz
    static const quint16 ANT_PLUS_CHANNEL_FREQUENCY = 2457; // Mhz

    /** message ids (in alphabetic order) */
    enum AntMessageId {
        ASSIGN_CHANNEL = 0x42,
        BROADCAST_EVENT = 0x4e,
        CHANNEL_EVENT = 0x40,
        OPEN_CHANNEL = 0x4b,
        REQUEST_MESSAGE = 0x4d,
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

    /** Create an AntMessage2 from bytes. The bytes should contain the complete message.
     * @return an invalid (null) unique_ptr if bytes did not contain a valid message
     */
    static std::unique_ptr<AntMessage2> createMessageFromBytes(const QByteArray& bytes);

    // static factory methods for different messages
    static AntMessage2 assignChannel(quint8 channelNumber, quint8 channelType = 0, quint8 networkNumber = ANT_PLUS_NETWORK_NUMBER);
    static AntMessage2 openChannel(quint8 channelNumber);
    static AntMessage2 requestMessage(quint8 channelNumber, AntMessageId messageId);
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
    /**
     * @brief Construct the ANT message to set the ant network key.
     * @param networkNumber the network to set the key for.
     * @param networkKey the network key, an 8 byte array.
     * @return the message.
     */
    static AntMessage2 setNetworkKey(quint8 networkNumber, const std::array<quint8, 8> &networkKey);
    static AntMessage2 setSearchTimeout(quint8 channelNumber, int seconds);
    static AntMessage2 systemReset();
    static AntMessage2 unassignChannel(quint8 channelNumber);

    /**
     * @brief return a pointer to this AntMessage as an AntChannelEventMessage. This fails (with an assert) if the
     * message is not an AntChannelEventMessage.
     * @return the message as an AntChannelEventMessage.
     */
    const AntChannelEventMessage* asChannelEventMessage() const;

    quint8 contentByte(int nr) const;
    quint16 contentShort(int index) const;

protected:
    AntMessage2(const QByteArray& completeMessageBytes);
    AntMessage2(const AntMessageId id, const QByteArray& content);
    const QByteArray& content() const;

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
        EVENT_RESPONSE_NO_ERROR = 0x00,
        EVENT_CHANNEL_RX_FAIL = 0x02,
        EVENT_CHANNEL_RX_SEARCH_TIMEOUT = 0x01
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

class HeartRateMessage;
class PowerMessage;
class SpeedAndCadenceMessage;
class BroadCastMessage
{
public:
    BroadCastMessage(const AntMessage2& antMessage);

    quint8 channelNumber() const;
    quint8 dataPage() const;
    const AntMessage2 &antMessage() const;


    HeartRateMessage toHeartRateMessage() const;
    PowerMessage toPowerMessage() const;
    SpeedAndCadenceMessage toSpeedAndCadenceMessage() const;
protected:
    AntMessage2 _antMessage;
private:
    quint8 _channelNumber;
    quint8 _dataPage;
};

/**
 * HeartRateMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ Heart Rate profile.
 * All Data Pages hold 8 bytes of content.
 *
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 * Byte 2-4 are different for different pages. We will not use them here.
 * Bytes 5 & 6 contain the measurement time in 1/1024s as an unsigned short (16 bits). (this wraps every 64 seconds)
 * Byte 7 contains the a count of heart beat events. This wraps every 255 counts. It can be used to check for missed
 * events.
 * Byte 8 contains the heart rate as computed by the sensor.
 */
class HeartRateMessage: public BroadCastMessage
{
public:
    /**
     * Create a HeartRateMessage
     * @param antMessage the ANT+ broadcast message.
     * @return An HeartRateMessage.
     */
    HeartRateMessage(const AntMessage2& antMessage);

    quint16 measurementTime() const;
    quint8 heartBeatCount() const;
    quint8 computedHeartRate() const;
private:
    quint16 _measurementTime;
    quint8 _heartBeatCount;
    quint8 _computedHeartRate;
};

/** Power message
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 *
 * For the Power-only page (0x10):
 * Byte 2 is the event count
 * Byte 3 pedal power (power balance)
 * Byte 4 instantaneous cadence
 * Byte 5-6 Accumulated Power
 * Byte 7-8 Instantaneous Power
*/
class PowerMessage: public BroadCastMessage
{
public:
    enum DataPages {
        POWER_ONLY_PAGE = 0x10
    };

    /** Create a PowerMessage.
     * @param antMessage the ANT+ broadcast message.
     * @return a PowerMessage
     */
    PowerMessage(const AntMessage2& antMessage);

    bool isPowerOnlyPage() const;

    quint8 eventCount() const;
    quint8 instantaneousCadence() const;
    quint16 accumulatedPower() const;
    quint16 instantaneousPower() const;
};

/** Speed And Cadence message
 * Byte 0 contains the channel
 *
 * Byte 1-2 cadence event time
 * Byte 3-4 cumulative cadence revolution count
 * Byte 5-6 bike speed event time
 * Byte 7-8 cumulative speed revolution count
*/
class SpeedAndCadenceMessage: public BroadCastMessage
{
public:
    SpeedAndCadenceMessage(const AntMessage2& antMessage);

    quint16 cadenceEventTime() const;
    quint16 pedalRevolutions() const;
    quint16 speedEventTime() const;
    quint16 wheelRevolutions() const;

};

#endif // ANTMESSAGE2_H
