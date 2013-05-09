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

namespace {
const quint8 READ_SIZE = 64;
}
namespace indoorcycling
{

UnixSerialUsbAnt::UnixSerialUsbAnt(QObject *parent) :
	AntDevice(parent)
{
	QFileInfoList entries = findUsbSerialDevices();

	foreach (const QFileInfo& entry, entries) {
		if (isGarminUsb1Stick(entry)) {
			qDebug() << "Garmin USB1 stick found";
			_deviceFileInfo = entry;
		}
	}
	if (isValid()) {
		openConnection();
	}
}

UnixSerialUsbAnt::~UnixSerialUsbAnt()
{
	if (isValid()) {
		tcflush(_nativeDeviceHandle, TCIOFLUSH); // clear out the garbage
		close(_nativeDeviceHandle);
	}
}

bool UnixSerialUsbAnt::isDevicePresent()
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

bool UnixSerialUsbAnt::isValid() const
{
	return !_deviceFileInfo.fileName().isEmpty();
}

int UnixSerialUsbAnt::numberOfChannels() const
{
	return 4;
}

int UnixSerialUsbAnt::writeBytes(QByteArray &bytes)
{
	int rc;
	int ibytes;

	ioctl(_nativeDeviceHandle, FIONREAD, &ibytes);

	// timeouts are less critical for writing, since vols are low
	rc= write(_nativeDeviceHandle, bytes.data(), bytes.size());

	if (rc != -1) tcdrain(_nativeDeviceHandle); // wait till its gone.

	ioctl(_nativeDeviceHandle, FIONREAD, &ibytes);
	return rc;
}

QByteArray UnixSerialUsbAnt::readBytes()
{
	QByteArray allBytes;
	while(true) {
		QByteArray readBytes(READ_SIZE, '\0');
		int result = read(_nativeDeviceHandle, readBytes.data(), READ_SIZE);
		if (result == -1 || result == 0)
			break;
		allBytes.append(readBytes.left(result));
	}

	return allBytes;
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
	if ((_nativeDeviceHandle=open(qPrintable(_deviceFileInfo.absoluteFilePath()),O_RDWR | O_NOCTTY | O_NONBLOCK)) == -1)
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
}
