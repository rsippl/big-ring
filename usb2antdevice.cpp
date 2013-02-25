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
	AntDevice(parent), _writeTransfer(NULL), _readTransfer(NULL), _readBuffer(READ_SIZE, '\0'), _readBusy(false), _wasAttached(false), _setupComplete(false)
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
	// read device buffer to clear it.
	readBytes();
	_setupComplete = true;
}

Usb2AntDevice::~Usb2AntDevice()
{
	if (_readTransfer)
		libusb_cancel_transfer(_readTransfer);
	if (_writeTransfer)
		libusb_cancel_transfer(_writeTransfer);
	// cancel all pending transfers.
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

	_bytesToWrite.append(bytes);

	if (_writeTransfer != NULL)
		return 0;

	if (!_readTransfer) {
	qDebug() << "issuing write";
	_writeBuffer = _bytesToWrite;
	_bytesToWrite.clear();
	_writeTransfer = libusb_alloc_transfer(0);

	libusb_fill_bulk_transfer(_writeTransfer, _deviceHandle, ENDPOINT_OUT, (unsigned char*) _writeBuffer.data(), _writeBuffer.size(),
							  Usb2AntDevice::writeCallback, this, 100);

	libusb_submit_transfer(_writeTransfer);
	qDebug() << "finished issuing write";
	}
	return 0;
}

QByteArray Usb2AntDevice::readBytes() {
	if (!_deviceHandle)
		return QByteArray();
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	libusb_handle_events_locked(_context, &tv);
	// no current read, make a new one.
	if (!_readTransfer) {
		qDebug() << "issueing read";
		_readTransfer = libusb_alloc_transfer(0);
		libusb_fill_bulk_transfer(_readTransfer, _deviceHandle, ENDPOINT_IN, (unsigned char*) _readBuffer.data(), _readBuffer.size(),
								  readCallback, this, 500);
		libusb_submit_transfer(_readTransfer);
	}
	QByteArray bytes(_bytesRead);
	_bytesRead.clear();
	return bytes;
}

void Usb2AntDevice::writeCallback(libusb_transfer *transfer)
{
	Usb2AntDevice* self = static_cast<Usb2AntDevice*>(transfer->user_data);
	self->writeReady(transfer);
}


void Usb2AntDevice::readCallback(libusb_transfer *transfer)
{
	Usb2AntDevice* self = static_cast<Usb2AntDevice*>(transfer->user_data);
	self->readReady(transfer);
}


void Usb2AntDevice::writeReady(libusb_transfer *)
{
	if (_writeTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
		qDebug() << "transfer completed";
	} else {
		qDebug() << "transfer error";
	}

	libusb_free_transfer(_writeTransfer);
	_writeTransfer = NULL;
	_writeBuffer.clear();
}

void Usb2AntDevice::readReady(libusb_transfer *)
{
	switch (_readTransfer->status) {
	case LIBUSB_TRANSFER_COMPLETED:
		qDebug() << "read transfer completed";
		break;
	default:
		qDebug() << "read status:" << _readTransfer->status << LIBUSB_TRANSFER_COMPLETED;
	}

	_bytesRead.append(_readBuffer.left(_readTransfer->actual_length));
	_readBuffer.fill('\0');

	libusb_free_transfer(_readTransfer);
	_readTransfer = NULL;
}
}
