#include "usb2antdevice.h"
#include <QtDebug>
namespace
{
const int INTERFACE_NR = 0;
const int ENDPOINT_IN = 0x81; /* endpoint 0x81 address for IN */
const int ENDPOINT_OUT = 0x01; /* endpoint 1 address for OUT */
const quint8 READ_SIZE = 64;
const int TIMEOUT = 10; // ms
}
namespace indoorcycling
{

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
	AntDevice(parent), _wasAttached(false), _setupComplete(false)
{
	libusb_init(&_context);
	libusb_set_debug(_context, 4);
	_deviceHandle = libusb_open_device_with_vid_pid(_context, GARMIN_USB_VENDOR_ID, GARMIN_USB2_PRODUCT_ID);
	if (!_deviceHandle) {
		qDebug() << "no device handle";
		return;
	}
#ifdef Q_OS_LINUX
	if (libusb_kernel_driver_active(_deviceHandle, INTERFACE_NR)) {
		qDebug() << "detaching kernel driver";
		int error = libusb_detach_kernel_driver(_deviceHandle, INTERFACE_NR);
		if (error) {
			qDebug() << "Unable to detach kernel driver" << libusb_error_name(error);
		}
		_wasAttached = true;
	}
#endif
	int err = libusb_claim_interface(_deviceHandle, 0);
	if (err){
		qDebug() << "Unable to claim interface" << libusb_error_name(err);
		return;
	}
	libusb_clear_halt(_deviceHandle, ENDPOINT_IN);
	libusb_clear_halt(_deviceHandle, ENDPOINT_OUT);
	_setupComplete = true;
}

Usb2AntDevice::~Usb2AntDevice()
{
	if (_deviceHandle) {
		libusb_release_interface(_deviceHandle, INTERFACE_NR);
		if (_wasAttached)
			libusb_attach_kernel_driver(_deviceHandle, INTERFACE_NR);
		libusb_close(_deviceHandle);
	}
	libusb_exit(_context);
}

bool Usb2AntDevice::isValid() const {
	return _setupComplete;
}

int Usb2AntDevice::numberOfChannels() const {
	return 8;
}

int Usb2AntDevice::writeBytes(QByteArray &bytes) {
	if (!_deviceHandle)
		return -1;

	QByteArray bytesLeftToWrite = bytes;

	int transferred;
	while(!bytesLeftToWrite.isEmpty()) {
		int error = libusb_bulk_transfer(_deviceHandle, ENDPOINT_OUT, reinterpret_cast<unsigned char*>(bytesLeftToWrite.data()),
										 bytesLeftToWrite.size(), &transferred, 100);
		if (error) {
			qDebug() << "usb write error" << libusb_error_name(error);
		}
		bytesLeftToWrite.remove(0, transferred);
	}
	return bytes.size();
}

QByteArray Usb2AntDevice::readBytes() {
	QByteArray allBytes;
	while(true) {
		QByteArray readBytes(READ_SIZE, '\0');
		int transferred;
		int error = libusb_bulk_transfer(_deviceHandle, ENDPOINT_IN, reinterpret_cast<unsigned char*>(readBytes.data()),
										 READ_SIZE, &transferred, 10);
		if (error && error != LIBUSB_ERROR_TIMEOUT) {
			qDebug() << "usb read error" << libusb_error_name(error);
		}
		if (transferred == 0)
			break;
		allBytes.append(readBytes.left(transferred));

	}
	return allBytes;
}
}
