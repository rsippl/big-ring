#ifndef ANTSPEEDANDCADENCECHANNELHANDLER_H
#define ANTSPEEDANDCADENCECHANNELHANDLER_H

#include <memory>
#include <QtCore/QVariant>

#include "antchannelhandler.h"

namespace indoorcycling
{
class AntSpeedAndCadenceChannelHandler : public AntChannelHandler
{
public:
    virtual ~AntSpeedAndCadenceChannelHandler() {}
    /**
     * Create a new combined ANT+ speed and cadence handling channel.
     * @return
     */
    static std::unique_ptr<AntSpeedAndCadenceChannelHandler> createCombinedSpeedAndCadenceChannelHandler (int channelNumber);
    static std::unique_ptr<AntSpeedAndCadenceChannelHandler> createCadenceChannelHandler (int channelNumber);
    static std::unique_ptr<AntSpeedAndCadenceChannelHandler> createSpeedChannelHandler (int channelNumber);
protected:
    virtual void handleBroadCastMessage(const BroadCastMessage &message) override;
private:
    AntSpeedAndCadenceChannelHandler(int channelNumber, AntSensorType sensorType, AntSportPeriod period);

    void handleSpeedAndCadenceMessage(const SpeedAndCadenceMessage& message);
    void handleCadenceMessage(const CadenceMessage& message);
    void handleSpeedMessage(const SpeedMessage& message);
    void calculateSpeed(const quint16 previousTime, const quint16 previousWheelRevolutions,
                        const quint16 currentTime, const quint16 currentWheelRevolutions);
    void calculateCadence(const quint16 previousTime, const quint16 previousPedalRevolutions,
                        const quint16 currentTime, const quint16 currentPedalRevolutions);
    BroadCastMessage _previousMessage;

};
}
#endif // ANTSPEEDANDCADENCECHANNELHANDLER_H
