#ifndef ANTDEVICEFINDER_H
#define ANTDEVICEFINDER_H

#include <QObject>
#include <QSharedPointer>
#include "antdevice.h"
struct libusb_context;

namespace indoorcycling {

class AntDeviceFinder : public QObject
{
	Q_OBJECT
public:
	explicit AntDeviceFinder(QObject *parent = 0);
	virtual ~AntDeviceFinder();
	
	AntDeviceType findAntDevice();

	/** Open an AntDevice. Returns an invalid pointer if no device can be found. */
	QSharedPointer<AntDevice> openAntDevice();
private:
	libusb_context* _context;
};
}
#endif // ANTDEVICEFINDER_H
