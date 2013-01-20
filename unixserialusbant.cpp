#include "unixserialusbant.h"
#include <QDirIterator>
#include <QStringList>
#include <QtDebug>

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

UnixSerialUsbAnt::UnixSerialUsbAnt(QObject *parent) :
	QObject(parent)
{
	QFileInfoList entries = findUsbSerialDevices();

	foreach (const QFileInfo& entry, entries) {
		if (isGarminUsb1Stick(entry)) {
			qDebug() << "Garmin USB1 stick found";
			_deviceFileInfo = entry;
		}
	}
	if (isValid()) {
		int rc = openConnection();
		qDebug() << "rc=" << rc;
	}
}

UnixSerialUsbAnt::~UnixSerialUsbAnt()
{
	if (isValid()) {
		tcflush(_nativeDeviceHandle, TCIOFLUSH); // clear out the garbage
		close(_nativeDeviceHandle);
	}
}

bool UnixSerialUsbAnt::isAntUsb1StickPresent()
{
	QFileInfoList entries = findUsbSerialDevices();

	foreach (const QFileInfo& entry, entries) {
		if (isGarminUsb1Stick(entry)) {
			qDebug() << "Garmin USB1 stick found";
			return true;
		}
	}

	return false;
}

bool UnixSerialUsbAnt::isValid()
{
	return !_deviceFileInfo.fileName().isEmpty();
}

int UnixSerialUsbAnt::writeBytes(quint8 *bytes, int size)
{
	int rc;
	int ibytes;
	qDebug() << "native device handle" << _nativeDeviceHandle;
	ioctl(_nativeDeviceHandle, FIONREAD, &ibytes);

	// timeouts are less critical for writing, since vols are low
	rc= write(_nativeDeviceHandle, bytes, size);

	if (rc != -1) tcdrain(_nativeDeviceHandle); // wait till its gone.

	ioctl(_nativeDeviceHandle, FIONREAD, &ibytes);
	return rc;
}

int UnixSerialUsbAnt::readBytes(quint8 *bytes, int size)
{
	int rc;
	int i=0;
	quint8 byte;

	// read one byte at a time sleeping when no data ready
	// until we timeout waiting then return error
	for (i=0; i<size; i++) {
		rc = read(_nativeDeviceHandle, &byte, 1);
		if (rc == -1 || rc == 0) return -1; // error!
		else bytes[i] = byte;
	}
	return i;
}

QFileInfoList UnixSerialUsbAnt::findUsbSerialDevices()
{
	QStringList filters;
	filters << "ttyUSB[0-9]";
	QDir dir("/dev");
	return dir.entryInfoList(filters, QDir::System);
}

bool UnixSerialUsbAnt::isGarminUsb1Stick(const QFileInfo& fileInfo)
{
	// All we can do for USB1 sticks is see if the cp210x driver module
	// is loaded for this device, and if it is, we will use the device
	// XXX need a better way of probing this device, but USB1 sticks
	//     are getting rarer, so maybe we can just make do with this
	//     until we deprecate them altogether
	struct stat s;
	if (stat(fileInfo.absoluteFilePath().toLatin1(), &s) == -1) return false;
	int maj = major(s.st_rdev);
	int min = minor(s.st_rdev);
	QString sysFile = QString("/sys/dev/char/%1:%2/device/driver/module/drivers/usb:cp210x").arg(maj).arg(min);
	if (QFileInfo(sysFile).exists()) return true;
	sysFile = QString("/sys/dev/char/%1:%2/device/driver/module/drivers/usb-serial:cp210x").arg(maj).arg(min);
	if (QFileInfo(sysFile).exists()) return true;

	return false;
}

int UnixSerialUsbAnt::openConnection()
{
	int ldisc=N_TTY; // LINUX
	if ((_nativeDeviceHandle=open(_deviceFileInfo.absoluteFilePath().toAscii(),O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)
		return errno;

	tcflush(_nativeDeviceHandle, TCIOFLUSH); // clear out the garbage

	if (ioctl(_nativeDeviceHandle, TIOCSETD, &ldisc) == -1) return errno;

	// get current settings for the port
	struct termios deviceSettings;
	tcgetattr(_nativeDeviceHandle, &deviceSettings);

	// set raw mode i.e. ignbrk, brkint, parmrk, istrip, inlcr, igncr, icrnl, ixon
	//                   noopost, cs8, noecho, noechonl, noicanon, noisig, noiexn
	cfmakeraw(&deviceSettings);
	cfsetspeed(&deviceSettings, B115200);

	// further attributes
	deviceSettings.c_iflag= IGNPAR;
	deviceSettings.c_oflag=0;
	deviceSettings.c_cflag &= (~CSIZE & ~CSTOPB);
#if defined(Q_OS_MACX)
	deviceSettings.c_cflag |= (CS8 | CREAD | HUPCL | CCTS_OFLOW | CRTS_IFLOW);
#else
	deviceSettings.c_cflag |= (CS8 | CREAD | HUPCL | CRTSCTS);
#endif
	deviceSettings.c_lflag=0;
	deviceSettings.c_cc[VMIN]=0;
	deviceSettings.c_cc[VTIME]=0;

	// set those attributes
	if(tcsetattr(_nativeDeviceHandle, TCSANOW, &deviceSettings) == -1) return errno;
	tcgetattr(_nativeDeviceHandle, &deviceSettings);

	// success
	return 0;
}
