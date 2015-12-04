#ifndef ANTSMARTTRAINERCHANNELHANDLER_H
#define ANTSMARTTRAINERCHANNELHANDLER_H
#include <QtCore/QTimer>
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
 * Byte 2 update event count
 * Byte 3 instantaneous cadence
 * Byte 4-5 accumulated power
 * Bytes 6-first 4 of 7 instantaneous power
 * Byte 7, last 4 bits. Trainer status
 * Byte 8 Flags & state
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

    bool bicyclePowerCalibrationNeeded() const;
    bool resistanceCalibrationNeeded() const;
    bool userConfigurationNeeded() const;

private:
    quint8 _eventCount;
    quint8 _cadence;
    quint16 _instantaneousPower;
    bool _bicyclePowerCalibrationNeeded;
    bool _resistanceCalibrationNeeded;
    bool _userConfigurationNeeded;
};

class CommandStatusReplyMessage;
class UserConfigurationMessage;

class AntSmartTrainerChannelHandler : public AntChannelHandler
{
    Q_OBJECT
public:
    enum class DataPage {
        GENERAL_FITNESS_EQUIPMENT = 0x10,
        SPECIFIC_TRAINER_DATA = 0x19,
        WIND_RESISTANCE = 0x32,
        TRACK_RESISTANCE = 0x33,
        USER_CONFIGURATION = 0x37,
        DATA_PAGE_REQUEST = 0x46,
        COMMAND_STATUS = 0x47
    };

    explicit AntSmartTrainerChannelHandler(int channelNumber, QObject *parent = nullptr);

    void setWeight(const qreal userWeightInKilograms, const qreal bikeWeightInKilograms);
    void setSlope(const qreal slopeInPercent);
protected:
    virtual void channelOpened() override final;
    virtual void handleBroadCastMessage(const BroadCastMessage &message) override;
private slots:
    void checkConfiguration();
private:
    void handleGeneralFitnessEquipmentMessage(const GeneralFitnessEquipmentMessage& message);
    void handleSpecificTrainerDataMessage(const SpecificTrainerDataMessage &message);
    void handleCommandStatusReplyMessage(const CommandStatusReplyMessage &message);
    void handleUserConfigurationMessage(const UserConfigurationMessage &message);

    AntMessage2 createWindResistenceMessage();
    AntMessage2 createTrackResistanceMessage();
    AntMessage2 createUserConfigurationMessage();
    AntMessage2 createRequestMessage(const DataPage dataPage);

    qreal _slope = 0.0; // TODO: remove random value.
    qreal _bikeWeight = 10.0;
    qreal _userWeight = 75.0;

    QTimer *_configurationCheckerTimer;
};
}
#endif // ANTSMARTTRAINERCHANNELHANDLER_H
