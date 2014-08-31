#ifndef USBEXPRESSANTDEVICE_H
#define USBEXPRESSANTDEVICE_H

#include "antdevice.h"
#include <QObject>
#include <windows.h>
extern "C" {
#include <SiUSBXp.h>
}

namespace indoorcycling
{
class UsbExpressAntDevice: public AntDevice
{
	Q_OBJECT
public:
	UsbExpressAntDevice(QObject* parent = 0);
	virtual ~UsbExpressAntDevice();

	static bool isDevicePresent();
	virtual int numberOfChannels() const;
	virtual bool isValid() const;
	virtual int writeBytes(QByteArray &bytes);
	virtual QByteArray readBytes();

private:
	static int findDevice();
	bool openConnection();

	Qt::HANDLE _deviceHandle;
};
}
#endif // USBEXPRESSANTDEVICE_H
