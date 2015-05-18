#ifndef ANTPOWERCHANNELHANDLER_H
#define ANTPOWERCHANNELHANDLER_H

#include "antchannelhandler.h"
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

    bool isPowerOnlyPage() const;

    quint8 eventCount() const;
    quint8 instantaneousCadence() const;
    quint16 accumulatedPower() const;
    quint16 instantaneousPower() const;
};

class AntPowerChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntPowerChannelHandler(int channelNumber, QObject* parent);
    virtual ~AntPowerChannelHandler() {}
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage& message) override;
private:
    PowerMessage _lastPowerMessage;
};
}
#endif // ANTPOWERCHANNELHANDLER_H
