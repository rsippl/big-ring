#include "antsmarttrainerchannelhandler.h"

#include <QtCore/QtDebug>
namespace {
enum class DataPage {
    GENERAL_FITNESS_EQUIPMENT = 0x10,
    SPECIFIC_TRAINER_DATA = 0x19,
    WIND_RESISTANCE = 0x32,
    TRACK_RESISTANCE = 0x33,
    USER_CONFIGURATION = 0x37
};

const quint8 TRAINER_EQUIPMENT_TYPE = 25;

}
namespace indoorcycling {
AntSmartTrainerChannelHandler::AntSmartTrainerChannelHandler(int channelNumber, QObject *parent) :
    AntChannelHandler(channelNumber, AntSensorType::SMART_TRAINER, AntSportPeriod::SMART_TRAINER, parent)
{
    // empty
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
    default:
        qWarning("Unknown message %d", message.dataPage());
    }
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
}

void AntSmartTrainerChannelHandler::handleGeneralFitnessEquipmentMessage(const GeneralFitnessEquipmentMessage &message)
{
    qDebug() << "elapsed time:" << message.elapsedTime() << "distance:" << message.distanceTravelled() << "speed" << message.instantaneousSpeedMmps();
}

void AntSmartTrainerChannelHandler::handleSpecificTrainerDataMessage(const SpecificTrainerDataMessage &message)
{
    qDebug() << "Trainer Data event count:" << message.eventCount() << "cadence" << message.cadence() << "power" << message.instantaneousPower();
    emit sensorValue(SensorValueType::POWER_WATT, AntSensorType::SMART_TRAINER, QVariant::fromValue(message.instantaneousPower()));
    emit sensorValue(SensorValueType::CADENCE_RPM, AntSensorType::CADENCE, QVariant::fromValue(message.cadence()));
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

    qint16 slopeAsInt = static_cast<qint16>(std::round((_slope + 200) * 100));
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

    content += 0xFF; // invalid wheel size (do  we need it?)
    quint8 invalidGearRatio = 0x0;
    content += invalidGearRatio; // invalid gear ratio. Not needed.

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

}
