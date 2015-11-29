#include "antsmarttrainerchannelhandler.h"

#include <QtCore/QtDebug>
namespace {
enum class DataPage {
    GENERAL_FITNESS_EQUIPMENT = 0x10,
    SPECIFIC_TRAINER_DATA = 0x19
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

AntMessage2 AntSmartTrainerChannelHandler::createWindResistenceMessage()
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(0x32);
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += static_cast<quint8>(0xFF); // default value
    content += static_cast<quint8>(0xFF); // default value
    content += static_cast<quint8>(0xFF); // default value

    return AntMessage2(AntMessage2::AntMessageId::ACKNOWLEDGED_MESSAGE, content);
}

AntMessage2 AntSmartTrainerChannelHandler::createTrackResistanceMessage()
{
    QByteArray content;
    content += channelNumber();
    content += static_cast<quint8>(0x33);
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved
    content += 0xFF; // reserved

    qint16 slopeAsInt = static_cast<qint16>(std::round((_slope + 200) * 100));
    quint8 lsb = slopeAsInt & 0xFF;
    quint8 msb = (slopeAsInt >> 8) & 0xFF;
    content += lsb;
    content += msb;
    content += 0xFF; // default value;

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
