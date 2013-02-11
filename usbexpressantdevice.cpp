#include "usbexpressantdevice.h"

#include <QtDebug>

namespace {
const quint16 GARMIN_USB1_PID = 0x0fcf;
const quint16 GARMIN_USB1_VID = 0x1004;

const quint8 READ_SIZE = 64;
}
UsbExpressAntDevice::UsbExpressAntDevice(QObject *parent):
	AntDevice(parent), _deviceHandle(0)
{
	if (!openConnection())
		qDebug() << "Unable to open connection";
}

UsbExpressAntDevice::~UsbExpressAntDevice()
{
	if (_deviceHandle)
		SI_Close(_deviceHandle);
}

bool UsbExpressAntDevice::isDevicePresent()
{
	return (findDevice() != -1);
}

bool UsbExpressAntDevice::isValid()
{
	return (_deviceHandle);
}

int UsbExpressAntDevice::writeBytes(QByteArray &bytes)
{
	DWORD nrOfBytesWritten;
	SI_STATUS status = SI_Write(_deviceHandle, bytes.data(), bytes.size(), &nrOfBytesWritten);
	if (status == SI_SUCCESS)
		return nrOfBytesWritten;
	else {
		qDebug() << "Unable to write so usb device.";
		return -1;
	}
}

QByteArray UsbExpressAntDevice::readBytes()
{
	QByteArray allBytes;
	while(true) {
		QByteArray readBytes(READ_SIZE, '\0');
		DWORD nrOfBytesRead;
		SI_STATUS status = SI_Read(_deviceHandle, readBytes.data(), READ_SIZE, &nrOfBytesRead);
		if (status != SI_SUCCESS || nrOfBytesRead == 0)
			break;
		allBytes.append(readBytes.left(nrOfBytesRead));
	}
	return allBytes;
}

int UsbExpressAntDevice::findDevice()
{
	DWORD numDevices;

	// any USBXpress devices connected?
	SI_GetNumDevices(&numDevices);

	// lets see if one of them is a GARMIN USB1 stick and open it
	for (unsigned int deviceNr=0; deviceNr<numDevices; deviceNr++) {

		char buffer[128];
		bool vidok, pidok;

		// we want product 1004 and vendor 0fcf
		SI_GetProductString (deviceNr, &buffer, SI_RETURN_PID);
		unsigned int vid = QString(buffer).toInt(&vidok, 16);
		SI_GetProductString (deviceNr, &buffer, SI_RETURN_VID);
		unsigned int pid = QString(buffer).toInt(&pidok, 16);

		// we found ours?
		if (vidok && vid == GARMIN_USB1_VID && pidok && pid == GARMIN_USB1_PID) {
			qDebug() << "Garmin USB Stick found.";
			return deviceNr;
		}
	}

	qDebug() << "No Garmin USB Stick found.";
	return -1;
}

bool UsbExpressAntDevice::openConnection()
{
	int deviceNr = findDevice();

	if (deviceNr == -1) {
		qDebug() << "No Garmin USB Stick found.";
		return false;
	}

	if (SI_Open(deviceNr, &_deviceHandle) == SI_SUCCESS) {

		// clear out any crap
		SI_FlushBuffers(_deviceHandle, 1, 1);
		SI_SetTimeouts(5,5); // 5ms timeout seems ok
		SI_SetBaudRate(_deviceHandle, 115200); // USB1 supports this
		SI_SetLineControl(_deviceHandle, 0x800); // 8bit, 1 stop, no parity
		SI_SetFlowControl(_deviceHandle, /*bCTS_MaskCode */ SI_STATUS_INPUT,
						  /*bRTS_MaskCode */ SI_HELD_INACTIVE,
						  /*bDTR_MaskCode */ SI_HELD_INACTIVE,
						  /*bDSRMaskCode  */ SI_STATUS_INPUT,
						  /*bDCD_MaskCode */ SI_STATUS_INPUT,
						  /*bFlowXonXoff  */ FALSE);

		// success!
		return true;
	}
	return false;
}
