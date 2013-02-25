#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

#include <QList>
#include <QSharedPointer>

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
	static void writeCallback(libusb_transfer* transfer);
	static void readCallback(libusb_transfer* transfer);

	void writeReady(libusb_transfer* transfer);
	void readReady(libusb_transfer* transfer);

	libusb_context* _context;
	libusb_device_handle* _deviceHandle;
	libusb_transfer* _writeTransfer;
	libusb_transfer* _readTransfer;
	QByteArray _writeBuffer;
	QByteArray _readBuffer;

	QByteArray _bytesRead;
	QByteArray _bytesToWrite;
	bool _readBusy;
	bool _wasAttached;
	bool _setupComplete;
};
}
#endif // USB2ANTDEVICE_H
