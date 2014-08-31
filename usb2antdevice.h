#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

#include <QList>
#include <QTimer>

extern "C" {
#include <libusb-1.0/libusb.h>
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

private slots:
	void doTransfer();

private:
	/** callback functions */
	static void writeCallback(libusb_transfer* transfer);
	static void readCallback(libusb_transfer* transfer);
	void writeReady(libusb_transfer* transfer);
	void readReady(libusb_transfer* transfer);

	/** internal read and write functions */
	void doWrite();
	void doRead();

	libusb_context* _context;
	libusb_device_handle* _deviceHandle;
	libusb_transfer* _currentTransfer;

	QByteArray _writeBuffer;
	QByteArray _readBuffer;

	QTimer *_transferTimer;
	QByteArray _bytesRead;
	QList<QByteArray> _messagesToWrite;
	bool _wasAttached;
	bool _setupComplete;
};
}
#endif // USB2ANTDEVICE_H
