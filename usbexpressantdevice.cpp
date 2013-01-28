#include "usbexpressantdevice.h"

namespace {
const quint16 GARMIN_USB1_PID = 0x0fcf;
const quint16 GARMIN_USB1_VID = 0x1004;
}
UsbExpressAntDevice::UsbExpressAntDevice(QObject *parent):
	AntDevice(parent)
{
}

bool UsbExpressAntDevice::isDevicePresent()
{
	DWORD numDevices;

	// any USBXpress devices connected?
	SI_GetNumDevices(&numDevices);
	if (numDevices == 0) return -1;

	// lets see if one of them is a GARMIN USB1 stick and open it
	for (unsigned int i=0; i<numDevices; i++) {

		char buffer[128];
		bool vidok, pidok;

		// we want product 1004 and vendor 0fcf
		SI_GetProductString (i, &buffer, SI_RETURN_PID);
		unsigned int vid = QString(buffer).toInt(&vidok, 16);
		SI_GetProductString (i, &buffer, SI_RETURN_VID);
		unsigned int pid = QString(buffer).toInt(&pidok, 16);

		// we found ours?
		if (vidok && vid == GARMIN_USB1_VID && pidok && pid == GARMIN_USB1_PID) {
			return true;
		}
	}

	// no dice. fail.
	return false;
}

bool UsbExpressAntDevice::isValid()
{
	return false;
}

int UsbExpressAntDevice::writeBytes(QByteArray &)
{
	return -1;
}

QByteArray UsbExpressAntDevice::readBytes()
{
	return QByteArray();
}
