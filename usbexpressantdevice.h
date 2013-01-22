#ifndef USBEXPRESSANTDEVICE_H
#define USBEXPRESSANTDEVICE_H

#include "antdevice.h"
#include <QObject>
#include <windows.h>
extern "C" {
#include <SiUSBXp.h>
}
class UsbExpressAntDevice: public AntDevice
{
    Q_OBJECT
public:
    UsbExpressAntDevice(QObject* parent = 0);
    virtual ~UsbExpressAntDevice() {}

    static bool isDevicePresent();
    virtual bool isValid();
    virtual int writeBytes(QByteArray &bytes);
    virtual QByteArray readBytes();
};

#endif // USBEXPRESSANTDEVICE_H
