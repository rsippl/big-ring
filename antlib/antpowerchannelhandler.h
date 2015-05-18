#ifndef ANTPOWERCHANNELHANDLER_H
#define ANTPOWERCHANNELHANDLER_H

#include "antchannelhandler.h"
#include <QtCore/QTimer>
namespace indoorcycling
{
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
    PowerMessage(const AntMessage2& antMessage = AntMessage2());

    static AntMessage2 createPowerMessage(quint8 channel, quint8 eventCount,
                                           quint16 accumulatedPower, quint16 instantaneousPower);

    bool isPowerOnlyPage() const;

    quint8 eventCount() const;
    quint8 instantaneousCadence() const;
    quint16 accumulatedPower() const;
    quint16 instantaneousPower() const;

    QByteArray toContentBytes() const;
};

class AntPowerReceiveChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerReceiveChannelHandler(int channelNumber, QObject* parent = nullptr);
    virtual ~AntPowerReceiveChannelHandler() {}
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    PowerMessage _lastPowerMessage;
};

class AntPowerTransmissionChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerTransmissionChannelHandler(int channelNumber, QObject* parent = nullptr);
    virtual ~AntPowerTransmissionChannelHandler() {}
public slots:
    void setPower(quint16 power);
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
    virtual bool isMasterNode() const override;
    virtual quint8 transmissionType() const override;
    virtual void channelOpened() override;
private slots:
    void sendPowerUpdate();
private:
    QTimer* _updateTimer;
    PowerMessage _lastPowerMessage;

    quint8 _eventCount;
    quint16 _accumulatedPower;
    quint16 _instantaneousPower;
};
}
#endif // ANTPOWERCHANNELHANDLER_H
