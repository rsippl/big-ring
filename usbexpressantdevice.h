#ifndef USBEXPRESSANTDEVICE_H
#define USBEXPRESSANTDEVICE_H

#include "antdevice.h"
#include <QObject>
class UsbExpressAntDevice: public AntDevice
{
    Q_OBJECT
public:
    UsbExpressAntDevice(QObject* parent = 0);
    virtual ~UsbExpressAntDevice() {}
};

#endif // USBEXPRESSANTDEVICE_H
