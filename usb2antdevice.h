#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

extern "C" {
#include <libusb.h>
}

namespace indoorcycling
{

class Usb2AntDevice : public AntDevice
{
	Q_OBJECT
public:
	explicit Usb2AntDevice(QObject *parent = 0);
	virtual ~Usb2AntDevice();
	virtual bool isValid() const;
	virtual int numberOfChannels() const;
	virtual int writeBytes(QByteArray& bytes);
	virtual QByteArray readBytes();

private:
	libusb_context* _context;
	libusb_device_handle* _deviceHandle;
	bool _wasAttached;
	bool _setupComplete;
};
}
#endif // USB2ANTDEVICE_H
