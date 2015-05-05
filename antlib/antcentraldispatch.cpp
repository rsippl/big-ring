#include "antcentraldispatch.h"

#include "antdevicefinder.h"
#include "antmessage2.h"
#include "antmessagegatherer.h"

#include <QtCore/QtDebug>

namespace {
const int INITIALIZATION_TIMEOUT = 3000; // ms

// According to the ANT specification, we should wait at least 500ms after sending
// the System Reset message. To be on the safe side, we'll wait 600ms.
const int ANT_RESET_SYSTEM_TIMEOUT = 600; // ms.

// ANT+ Network
const int ANT_PLUS_NETWORK_NUMBER = 1;
// ANT+ Network Key
const std::array<quint8,8> ANT_PLUS_NETWORK_KEY = { 0xB9, 0xA5, 0x21, 0xFB, 0xBD, 0x72, 0xC3, 0x45 };
}
namespace indoorcycling {

AntCentralDispatch::AntCentralDispatch(QObject *parent) :
    QObject(parent), _initialized(false), _antMessageGatherer(new AntMessageGatherer(this)), _initializationTimer(new QTimer(this))
{
    connect(_antMessageGatherer, &AntMessageGatherer::antMessageReceived, this,
            &AntCentralDispatch::messageFromAntUsbStick);
    _initializationTimer->setInterval(INITIALIZATION_TIMEOUT);
    _initializationTimer->setSingleShot(true);
    connect(_initializationTimer, &QTimer::timeout, _initializationTimer, [this]() {
        emit initializationFinished(false);
    });
}

bool AntCentralDispatch::antUsbStickPresent() const
{
    return _antUsbStick.get();
}

bool AntCentralDispatch::initialized() const
{
    return _initialized;
}

void AntCentralDispatch::initialize()
{
    _initializationTimer->start();
    scanForAntUsbStick();
    if (!_antUsbStick) {
        emit initializationFinished(false);
        return;
    }
    if (_antUsbStick->isReady()) {
        resetAntSystem();
    } else {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::deviceReady,
                this, &AntCentralDispatch::resetAntSystem);
    }
}

void AntCentralDispatch::messageFromAntUsbStick(const QByteArray &bytes)
{
    std::unique_ptr<AntMessage2> antMessage = AntMessage2::createMessageFromBytes(bytes);
    switch(antMessage->id()) {
    case AntMessage2::CHANNEL_EVENT:
        handleChannelEvent(*antMessage->asChannelEventMessage());
        break;
    default:
        qDebug() << "unhandled ANT+ message" << antMessage->toString();
    }
}

void AntCentralDispatch::scanForAntUsbStick()
{
    AntDeviceFinder deviceFinder;
    _antUsbStick = deviceFinder.openAntDevice();
    if (_antUsbStick) {
        connect(_antUsbStick.get(), &indoorcycling::AntDevice::bytesRead, _antMessageGatherer,
                &AntMessageGatherer::submitBytes);
    }
    emit antUsbStickScanningFinished(_antUsbStick.get());
}

void AntCentralDispatch::resetAntSystem()
{
    sendAntMessage(AntMessage2::systemReset());
    QTimer::singleShot(ANT_RESET_SYSTEM_TIMEOUT, this, SLOT(sendNetworkKey()));
}

void AntCentralDispatch::sendNetworkKey()
{
    sendAntMessage(AntMessage2::setNetworkKey(ANT_PLUS_NETWORK_NUMBER, ANT_PLUS_NETWORK_KEY));
}

void AntCentralDispatch::sendAntMessage(const AntMessage2 &message)
{
    Q_ASSERT_X(_antUsbStick.get(), "AntCentralDispatch::sendAntMessage", "usb stick should be present.");
    if (_antUsbStick) {
        qDebug() << "Sending ANT Message:" << message.toString();
        _antUsbStick->writeAntMessage(message);
    }
}

void AntCentralDispatch::handleChannelEvent(const AntChannelEventMessage &channelEventMessage)
{
    if (channelEventMessage.messageId() == AntMessage2::SET_NETWORK_KEY) {
        _initializationTimer->stop();
        _initialized = (channelEventMessage.messageCode() == AntChannelEventMessage::EVENT_RESPONSE_NO_ERROR);
        emit initializationFinished(_initialized);
    } else {
        qDebug() << "Unhandled channel event" << channelEventMessage.toString();
    }
}

}

