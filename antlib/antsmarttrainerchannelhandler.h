#ifndef ANTSMARTTRAINERCHANNELHANDLER_H
#define ANTSMARTTRAINERCHANNELHANDLER_H

#include "antchannelhandler.h"
namespace indoorcycling {

/**
 * GeneralFitnessEquipmentMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ FE-C profile.
 * All Data Pages hold 8 bytes of content.
 *
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 * Byte 2 contains the fitness equipment. We'll issue a warning if this is not a ANT+ FE-C Bike Trainer.
 * Byte 3 Contains the elapsed time in .25 of a second.
 * Byte 4 Contains the distance travelled in meters.
 * Bytes 5-6 Contains the speed in mm per second.
 * Bytes 7-8 Are not used currently.
 */
class GeneralFitnessEquipmentMessage: public BroadCastMessage
{
public:
    /**
     * Create a GeneralFitnessEquipmentMessage from an AntMessage2.
     * @param antMessage the ANT+ broadcast message.
     * @return an GeneralFitnessEquipmentMessage
     */
    GeneralFitnessEquipmentMessage(const AntMessage2& antMessage = AntMessage2());

    /**
     * Represents the elapsed time of the training in milliseconds. Rolls over every 64000 milliseconds (64 seconds)
     */
    quint32 elapsedTime() const;

    /**
     * Represents the distance travelled in meters. Rolls over every 256 meters.
     */
    quint32 distanceTravelled() const;

    /**
     * The speed in millimeters per second.
     */
    quint32 instantaneousSpeedMmps() const;

private:
    quint32 _elapsedTime;
    quint32 _distanceTravelled;
    quint32 _speedMmps;
};

/**
 * GeneralFitnessEquipmentMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ FE-C profile.
 * All Data Pages hold 8 bytes of content.
 *
 * Byte 0 contains the channel
 * Byte 1 contains the data page.
 * Byte 2 contains the fitness equipment. We'll issue a warning if this is not a ANT+ FE-C Bike Trainer.
 * Byte 3 Contains the elapsed time in .25 of a second.
 * Byte 4 Contains the distance travelled in meters.
 * Bytes 5-6 Contains the speed in mm per second.
 * Bytes 7-8 Are not used currently.
 */
class SpecificTrainerDataMessage: public BroadCastMessage
{
public:
    /**
     * Create a SpecificTrainerDataMessage from an AntMessage2.
     * @param antMessage the ANT+ broadcast message.
     * @return an SpecificTrainerDataPage
     */
    SpecificTrainerDataMessage(const AntMessage2& antMessage = AntMessage2());

    /**
     * Event count.
     */
    quint8 eventCount() const;

    quint8 cadence() const;

    quint16 instantaneousPower() const;

private:
    quint8 _eventCount;
    quint8 _cadence;
    quint16 _instantaneousPower;
};


class AntSmartTrainerChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    explicit AntSmartTrainerChannelHandler(int channelNumber, QObject *parent = nullptr);

    virtual void handleBroadCastMessage(const BroadCastMessage &message) override;
private:
    void handleGeneralFitnessEquipmentMessage(const GeneralFitnessEquipmentMessage& message);
    void handleSpecificTrainerDataMessage(const SpecificTrainerDataMessage &message);
};
}
#endif // ANTSMARTTRAINERCHANNELHANDLER_H
