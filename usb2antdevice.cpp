#include "usb2antdevice.h"

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
	AntDevice(parent)
{
}

Usb2AntDevice::~Usb2AntDevice()
{
}

bool Usb2AntDevice::isValid() const {
	return false;
}

int Usb2AntDevice::numberOfChannels() const {
	return 8;
}

int Usb2AntDevice::writeBytes(QByteArray &) {
	return -1;
}

QByteArray Usb2AntDevice::readBytes() {
	return QByteArray();
}
