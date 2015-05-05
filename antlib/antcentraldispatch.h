#ifndef ANTCENTRALDISPATCH_H
#define ANTCENTRALDISPATCH_H

#include <memory>

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include "antdevice.h"



class AntChannelEventMessage;
class AntMessageGatherer;
class AntMessage2;

namespace indoorcycling {
/**
 * The Main ANT+ class that manages the connection with the ANT+ usb stick and through that stick, to the ANT+
 * sensors that are found.
 */
class AntCentralDispatch : public QObject
{
    Q_OBJECT
public:
    explicit AntCentralDispatch(QObject *parent = 0);

    bool antUsbStickPresent() const;
    bool initialized() const;
signals:
    /** signal emitted when scanning for an usb stick is finished. @param found indicates whether or not an ANT+ usb
     * stick was found.
     */
    void antUsbStickScanningFinished(bool found);
    /**
     * Signal emitted when the connection to the ANT+ stick and the ANT+ network has been set up. After this, we
     * can try to connect to or search for ANT+ sensors. If success is false, we have not been able to initialize the
     * ANT+ connection.
     */
    void initializationFinished(bool success);
public slots:
    /**
     * Initialize the connection to the ANT+ stick. After calling this, listen for the signal initializationFinished(bool)
     * to check if the connection was successful.
     */
    void initialize();
private slots:
    void messageFromAntUsbStick(const QByteArray& bytes);
    /**
     * Reset ANT+ USB Stick.
     */
    void resetAntSystem();
    /**
     * Set ANT+ Network Key
     */
    void sendNetworkKey();
private:
    /**
     * Start scanning for an ANT+ usb stick. When scanning is finished, antUsbStickScanningFinished(AntDeviceType) is emitted.
     */
    void scanForAntUsbStick();

    /**
     * Send a message to the USB ANT+ Stick.
     */
    void sendAntMessage(const AntMessage2& message);

    /**
     * handle a channel event message.
     */
    void handleChannelEvent(const AntChannelEventMessage& channelEventMessage);

    std::unique_ptr<AntDevice> _antUsbStick;
    bool _initialized;
    AntMessageGatherer* _antMessageGatherer;

    QTimer* _initializationTimer;
};
}

#endif // ANTCENTRALDISPATCH_H
