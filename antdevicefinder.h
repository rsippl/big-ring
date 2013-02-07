#ifndef ANTDEVICEFINDER_H
#define ANTDEVICEFINDER_H

#include <QObject>

struct libusb_context;

namespace indoorcycling {
enum AntDeviceType {
	ANT_DEVICE_NONE,
	ANT_DEVICE_USB_1,
	ANT_DEVICE_USB_2
};

const int GARMIN_USB_VENDOR_ID = 0x0fcf;
const int GARMIN_USB1_PRODUCT_ID = 0x1004;
const int GARMIN_USB2_PRODUCT_ID = 0x1008;

class AntDeviceFinder : public QObject
{
	Q_OBJECT
public:
	explicit AntDeviceFinder(QObject *parent = 0);
	virtual ~AntDeviceFinder();
	
	AntDeviceType findAntDevice();

private:
	libusb_context* _context;
};
}
#endif // ANTDEVICEFINDER_H
