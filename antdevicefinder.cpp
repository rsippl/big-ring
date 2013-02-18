#include "antdevicefinder.h"
#ifdef Q_OS_WIN
#include "usbexpressantdevice.h"
#else
#include "unixserialusbant.h"
#endif
#include "usb2antdevice.h"
#include <QtDebug>
extern "C" {
#include <libusb.h>
}
namespace indoorcycling {

AntDeviceFinder::AntDeviceFinder(QObject *parent) :
	QObject(parent)
{
	libusb_init(&_context);
}

AntDeviceFinder::~AntDeviceFinder()
{
	libusb_exit(_context);
}

AntDeviceType AntDeviceFinder::findAntDevice()
{
	AntDeviceType type = ANT_DEVICE_NONE;
	libusb_device** device_list;
	ssize_t nrOfDevices = libusb_get_device_list(_context, &device_list);
	for (int i = 0; i < nrOfDevices; ++i) {
		libusb_device* device = device_list[i];
		libusb_device_descriptor descriptor;
		libusb_get_device_descriptor(device, &descriptor);
		if (descriptor.idVendor == GARMIN_USB_VENDOR_ID) {
			if (descriptor.idProduct == GARMIN_USB2_PRODUCT_ID) {
				qDebug() << "found ANT+ USB2 device";
				type = ANT_DEVICE_USB_2;
			} else if (descriptor.idProduct == GARMIN_USB1_PRODUCT_ID) {
				qDebug() << "found ANT+ USB1 device";
				type = ANT_DEVICE_USB_1;
			}
		}
	}

	libusb_free_device_list(device_list, 1);
	return type;
}

QSharedPointer<AntDevice> AntDeviceFinder::openAntDevice()
{
	AntDeviceType type = findAntDevice();
	switch(type) {
	case ANT_DEVICE_USB_1:
#ifdef Q_OS_LINUX
		return QSharedPointer<AntDevice>(new UnixSerialUsbAnt);
#else
		return QSharedPointer<AntDevice>(new UsbExpressAntDevice);
#endif
	case ANT_DEVICE_USB_2:
		return QSharedPointer<AntDevice>(new Usb2AntDevice);
	default:
		return QSharedPointer<AntDevice>();
	}
}

}
