#include "usb2antdevice.h"
#include <QtDebug>
namespace
{
const int INTERFACE_NR = 0;
const int ENDPOINT_IN = 0x81; /* endpoint 0x81 address for IN */
const int ENDPOINT_OUT = 0x01; /* endpoint 1 address for OUT */
const quint8 READ_SIZE = 64;
const int TIMEOUT = 250; // ms
const int READ_INTERVAL = 10;
}
namespace indoorcycling
{

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
	AntDevice(parent), _currentTransfer(NULL), _readBuffer(READ_SIZE, '\0'),
	_transferTimer(new QTimer(this)), _wasAttached(false), _setupComplete(false)
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

	connect(_transferTimer, SIGNAL(timeout()), SLOT(doTransfer()));
	_transferTimer->setInterval(READ_INTERVAL);
	_transferTimer->start();

	// read device buffer to clear it.
	doRead();
	_setupComplete = true;
}

Usb2AntDevice::~Usb2AntDevice()
{
	if (_currentTransfer)
		libusb_cancel_transfer(_currentTransfer);
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

	_messagesToWrite.append(bytes);
	return 0;
}

void Usb2AntDevice::doWrite()
{
	_writeBuffer = _messagesToWrite.first();
	_messagesToWrite.removeFirst();
	_currentTransfer = libusb_alloc_transfer(0);

	libusb_fill_bulk_transfer(_currentTransfer, _deviceHandle, ENDPOINT_OUT, (unsigned char*) _writeBuffer.data(), _writeBuffer.size(),
							  Usb2AntDevice::writeCallback, this, TIMEOUT);

	libusb_submit_transfer(_currentTransfer);
}

QByteArray Usb2AntDevice::readBytes() {
	if (!_deviceHandle)
		return QByteArray();

	QByteArray copy(_bytesRead);
	_bytesRead.clear();
	return copy;
}

void Usb2AntDevice::doRead()
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	libusb_handle_events_locked(_context, &tv);
	_currentTransfer = libusb_alloc_transfer(0);
	libusb_fill_bulk_transfer(_currentTransfer, _deviceHandle, ENDPOINT_IN, (unsigned char*) _readBuffer.data(), _readBuffer.size(),
							  readCallback, this, TIMEOUT);
	libusb_submit_transfer(_currentTransfer);
}

void Usb2AntDevice::doTransfer()
{
	libusb_handle_events_completed(_context, NULL);
	if (_currentTransfer)
		return;

	if (_messagesToWrite.isEmpty())
		doRead();
	else
		doWrite();
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
	libusb_free_transfer(_currentTransfer);
	_currentTransfer = NULL;
	_writeBuffer.clear();
}

void Usb2AntDevice::readReady(libusb_transfer *)
{
	_bytesRead.append(_readBuffer.left(_currentTransfer->actual_length));
	_readBuffer.fill('\0');

	libusb_free_transfer(_currentTransfer);
	_currentTransfer = NULL;
}
}
