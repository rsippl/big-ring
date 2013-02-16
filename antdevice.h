#ifndef ANTDEVICE_H
#define ANTDEVICE_H

#include <QByteArray>
#include <QObject>

namespace indoorcycling
{

enum AntDeviceType {
	ANT_DEVICE_NONE,
	ANT_DEVICE_USB_1,
	ANT_DEVICE_USB_2
};

const int GARMIN_USB_VENDOR_ID = 0x0fcf;
const int GARMIN_USB1_PRODUCT_ID = 0x1004;
const int GARMIN_USB2_PRODUCT_ID = 0x1008;


class AntDevice : public QObject
{
	Q_OBJECT
public:
	AntDevice(QObject* parent = 0);
	virtual ~AntDevice() {}
	virtual bool isValid() const = 0;
	virtual int numberOfChannels() const = 0;
	virtual int writeBytes(QByteArray& bytes) = 0;
	virtual QByteArray readBytes() = 0;
};
}
#endif // ANTDEVICE_H
