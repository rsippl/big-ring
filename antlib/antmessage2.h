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

    enum AntMessageId {
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
private:
    AntMessage2(const AntMessageId id, const QByteArray& content);

    quint8 computeChecksum(const QByteArray& bytes) const;
    AntMessageId _id;

    QByteArray _content;
};


#endif // ANTMESSAGE2_H
