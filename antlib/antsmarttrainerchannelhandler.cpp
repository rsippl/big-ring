#include "antsmarttrainerchannelhandler.h"

#include <QtCore/QtDebug>
namespace {
const quint8 TRAINER_EQUIPMENT_TYPE = 25;

const int CONFIGURATION_CHECK_INTERVAL = 5000;
}
namespace indoorcycling {
/**
 * CommandStatusReplyMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ FE-C profile.
 * All Data Pages hold 8 bytes of content.
 */
class CommandStatusReplyMessage: public BroadCastMessage
{
public:
    enum class Status: quint8 {
        PASS, FAIL, NOT_SUPPORTED, REJECTED, PENDING
    };

    /**
     * Create a CommandStatusReplyMessage from an AntMessage2.
     * @param antMessage the ANT+ broadcast message.
     * @return a CommandStatusReplyMessage
     */
    CommandStatusReplyMessage(const AntMessage2& antMessage = AntMessage2());

    AntSmartTrainerChannelHandler::DataPage dataPage() const { return _dataPage; }
    quint8 sequenceNr() const { return _sequenceNr; }
    bool lastCommand() const { return _lastCommand; }
    bool received() const { return _sequenceNr != 0xFF; }
    Status status() const { return _status; }

private:
    AntSmartTrainerChannelHandler::DataPage _dataPage;
    quint8 _lastCommand;
    quint8 _sequenceNr;
    Status _status;
};

/**
 * CommandStatusReplyMessage is an ANT+ Broadcast message.
 *
 * There are several data pages in the ANT+ FE-C profile.
 * All Data Pages hold 8 bytes of content.
 */
class UserConfigurationMessage: public BroadCastMessage
{
public:
    enum class Status: quint8 {
        PASS, FAIL, NOT_SUPPORTED, REJECTED, PENDING
    };

    /**
     * Create a UserConfigurationMessage from an AntMessage2.
     * @param antMessage the ANT+ broadcast message.
     * @return a UserConfigurationMessage
     */
    UserConfigurationMessage(const AntMessage2& antMessage = AntMessage2());

    qreal userWeight() const { return _userWeight; }

private:
    qreal _userWeight;
};

AntSmartTrainerChannelHandler::AntSmartTrainerChannelHandler(int channelNumber, QObject *parent) :
    AntChannelHandler(channelNumber, AntSensorType::SMART_TRAINER, AntSportPeriod::SMART_TRAINER, parent),
    _configurationCheckerTimer(new QTimer(this))
{
    connect(_configurationCheckerTimer, &QTimer::timeout, this, &AntSmartTrainerChannelHandler::checkConfiguration);
    _configurationCheckerTimer->setInterval(CONFIGURATION_CHECK_INTERVAL);
}


void AntSmartTrainerChannelHandler::handleBroadCastMessage(const BroadCastMessage &message)
{
    DataPage dataPage = static_cast<DataPage>(message.dataPage());
    switch (dataPage) {
    case DataPage::GENERAL_FITNESS_EQUIPMENT:
        handleGeneralFitnessEquipmentMessage(GeneralFitnessEquipmentMessage(message.antMessage()));
        return;
    case DataPage::SPECIFIC_TRAINER_DATA:
        handleSpecificTrainerDataMessage(SpecificTrainerDataMessage(message.antMessage()));
        break;
//    case DataPage::TRACK_RESISTANCE:
//        handleTrackResistanceMessage(TrackResistanceMessage(message.antMessage()));
//        break;
    case DataPage::COMMAND_STATUS:
        qDebug() << "Command status reply received";
        handleCommandStatusReplyMessage(CommandStatusReplyMessage(message.antMessage()));
        break;
    default:
        qWarning("Unknown message %d", message.dataPage());
    }
}

void AntSmartTrainerChannelHandler::checkConfiguration()
{
    queueAcknowledgedMessage(createRequestMessage(DataPage::COMMAND_STATUS));
}

void AntSmartTrainerChannelHandler::setWeight(const qreal userWeightInKilograms, const qreal bikeWeightInKilograms)
{
    _userWeight = userWeightInKilograms;
    _bikeWeight = bikeWeightInKilograms;
    queueAcknowledgedMessage(createUserConfigurationMessage());
}

void AntSmartTrainerChannelHandler::setSlope(const qreal slopeInPercent)
{
    _slope = slopeInPercent;
    queueAcknowledgedMessage(createTrackResistanceMessage());
}

void AntSmartTrainerChannelHandler::channelOpened()
{
    queueAcknowledgedMessage(createWindResistenceMessage());
    queueAcknowledgedMessage(createTrackResistanceMessage());
    _configurationCheckerTimer->start();
}

void AntSmartTrainerChannelHandler::handleGeneralFitnessEquipmentMessage(const GeneralFitnessEquipmentMessage&)
{
    // nothing interesting here for now. We don't use speed measurements from the trainer.
}

void AntSmartTrainerChannelHandler::handleSpecificTrainerDataMessage(const SpecificTrainerDataMessage &message)
{
    emit sensorValue(SensorValueType::POWER_WATT, AntSensorType::SMART_TRAINER, QVariant::fromValue(message.instantaneousPower()));
    emit sensorValue(SensorValueType::CADENCE_RPM, AntSensorType::SMART_TRAINER, QVariant::fromValue(message.cadence()));

    if (message.userConfigurationNeeded()) {
        queueAcknowledgedMessage(createUserConfigurationMessage());
    }
}

void AntSmartTrainerChannelHandler::handleCommandStatusReplyMessage(const CommandStatusReplyMessage &message)
{
    if (message.received() && (message.dataPage() == DataPage::TRACK_RESISTANCE || message.dataPage() == DataPage::WIND_RESISTANCE)) {
        qDebug() << "We're in simulation mode!";
    } else {
        qWarning("Something is wrong, we're not in simulation mode, %d",  static_cast<quint8>(message.dataPage()));
        qWarning("Last received command id = %d, Sequence # = %d, Status = %d", message.received(), message.sequenceNr(), static_cast<quint8>(message.status()));
        queueAcknowledgedMessage(createRequestMessage(DataPage::WIND_RESISTANCE));
    }
}

void AntSmartTrainerChannelHandler::handleUserConfigurationMessage(const UserConfigurationMessage &message)
{
    if (qFuzzyCompare(message.userWeight(), _userWeight)) {
        qDebug() << "User weight correctly configured";
    } else {
        qWarning("User weight not correctly configured on device!");
    }
}

/**
 * A wind resistance message contains the following information.
 *
 * As long as we don't implement drafting or altitude effects, these values can be left on their default.
 *
 * 0: channel number
 * 1: data page number 0x32
 * 2-5: reserved, set to 0xFF
 * 6: wind resistant coefficient, 0xFF for default value.
 * 7: wind speed, 0xFF for default value.
 * 8: drafting factor, 0xFF for default value (which is 100).
 */
AntMessage2 AntSmartTrainerChannelHandler::createWindResistenceMessage()
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(DataPage::WIND_RESISTANCE);
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved

    return AntMessage2(AntMessage2::AntMessageId::ACKNOWLEDGED_MESSAGE, content);
}

/**
 * A track resistance message contains the following information.
 *
 * This message is used to send slope changes.
 *
 * 0: channel number
 * 1: data page number 0x33
 * 2-5: reserved, set to 0xFF
 * 6-7: slope in 1/100 of percent.
 * 8: rolling resistance factor, 0xFF for default value (which is 0.004).
 */
AntMessage2 AntSmartTrainerChannelHandler::createTrackResistanceMessage()
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(DataPage::TRACK_RESISTANCE);
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved

    // slope in 1/100 of percent. Range -200% to +200%, so first add 200 (%) to the slope.
    qreal boundedSlope = qBound(-200.0, _slope, 200.0);
    qint16 slopeAsInt = static_cast<qint16>(std::round((boundedSlope + 200) * 100));
    content += slopeAsInt & 0xFF;
    content += ((slopeAsInt >> 8) & 0xFF);
    content += 0xFF; // default value;

    return AntMessage2(AntMessage2::AntMessageId::ACKNOWLEDGED_MESSAGE, content);
}

/**
 * A user configuration message contains the following information:
 *
 * 0: channel number
 * 1: data page number 0x37
 * 2-3: user weight in dekagrams (10g).
 * 4-5 (first nibble): reserved 0xFF.
 * 5 (last nibble) - 6: bike weight in multiples of 50 grams.
 * 7: bicycle wheel diameter. 0xFF for default.
 * 8: gear ratio: 0x00 for invalid.
 * This message is sent at the start of a ride.
 */
AntMessage2 AntSmartTrainerChannelHandler::createUserConfigurationMessage()
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(DataPage::USER_CONFIGURATION);
    // user's weight in dekagrams (100g), in two bytes.
    quint16 userWeightAsInDekaGram = static_cast<quint16>(std::round(_userWeight * 100));
    content += (userWeightAsInDekaGram & 0xFF);
    content += ((userWeightAsInDekaGram >> 8) & 0xFF);

    content += 0xFF; // reserved

    // first nibble 0xF, followed by 1.5 bytes of bike weight. Bike weight is in multiples of 50g (0.05kg).
    quint16 bikeWeight = static_cast<quint16>(std::round(_bikeWeight * 20)); // 20 = 1 / 0.05

    quint8 leastSignificantWeightNibble = bikeWeight & 0xF;
    content += ((leastSignificantWeightNibble << 4) | 0x0F);
    quint8 bikeWeightMsb = ((bikeWeight >> 4) & 0xFF);
    content += bikeWeightMsb;

    content += 0xFF; // default wheel size (0.7m diameter, 700C, normal racing wheels.
    quint8 invalidGearRatio = 0x0;
    content += invalidGearRatio; // invalid gear ratio. Not needed.

    return AntMessage2(AntMessage2::AntMessageId::ACKNOWLEDGED_MESSAGE, content);
}

AntMessage2 AntSmartTrainerChannelHandler::createRequestMessage(DataPage dataPage)
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(DataPage::DATA_PAGE_REQUEST);
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // no descriptor
    content += 0xFF; // no descriptor
    content += 0x01; // transmit the page once.
    content += static_cast<quint8>(dataPage);
    content += static_cast<quint8>(0x1);

    return AntMessage2(AntMessage2::AntMessageId::ACKNOWLEDGED_MESSAGE, content);
}

GeneralFitnessEquipmentMessage::GeneralFitnessEquipmentMessage(const AntMessage2 &antMessage)
{
    quint8 equipmentTypeField = antMessage.contentByte(2) & 0x1F;

    if (equipmentTypeField != TRAINER_EQUIPMENT_TYPE) {
        qDebug() << "This is not a stationary bike trainer!" << equipmentTypeField << antMessage.contentByte(2);
    }

    quint16 elapsedTimeShort = antMessage.contentByte(3);
    _elapsedTime = elapsedTimeShort * 250;
    _distanceTravelled = antMessage.contentByte(4);
    _speedMmps = antMessage.contentShort(5);
}

quint32 GeneralFitnessEquipmentMessage::elapsedTime() const
{
    return _elapsedTime;
}

quint32 GeneralFitnessEquipmentMessage::distanceTravelled() const
{
    return _distanceTravelled;
}

quint32 GeneralFitnessEquipmentMessage::instantaneousSpeedMmps() const
{
    return _speedMmps;
}

SpecificTrainerDataMessage::SpecificTrainerDataMessage(const AntMessage2 &antMessage)
{
    _eventCount = antMessage.contentByte(2);
    _cadence = antMessage.contentByte(3);

    quint16 powerBytes = antMessage.contentShort(6);
    _instantaneousPower = powerBytes & 0x0FFF;

    quint8 statusByte = (antMessage.contentByte(7) & 0xF0) >> 4;
    _bicyclePowerCalibrationNeeded = statusByte & 0x1;
    _resistanceCalibrationNeeded = statusByte & 0x2;
    _userConfigurationNeeded = statusByte & 0x4;
}

quint8 SpecificTrainerDataMessage::eventCount() const
{
    return _eventCount;
}

quint8 SpecificTrainerDataMessage::cadence() const
{
    return _cadence;
}

quint16 SpecificTrainerDataMessage::instantaneousPower() const
{
    return _instantaneousPower;
}

bool SpecificTrainerDataMessage::bicyclePowerCalibrationNeeded() const
{
    return _bicyclePowerCalibrationNeeded;
}

bool SpecificTrainerDataMessage::resistanceCalibrationNeeded() const
{
    return _resistanceCalibrationNeeded;
}

bool SpecificTrainerDataMessage::userConfigurationNeeded() const
{
    return _userConfigurationNeeded;
}

CommandStatusReplyMessage::CommandStatusReplyMessage(const AntMessage2 &antMessage)
{
    _dataPage = static_cast<AntSmartTrainerChannelHandler::DataPage>(antMessage.contentByte(1));
    _lastCommand = antMessage.contentByte(2);
    _sequenceNr = antMessage.contentByte(3);
    _status = static_cast<Status>(antMessage.contentByte(4));
}

UserConfigurationMessage::UserConfigurationMessage(const AntMessage2 &antMessage)
{
    quint16 userWeightDekaGrams = antMessage.contentShort(2);
    _userWeight = userWeightDekaGrams * 0.01;
}

}
