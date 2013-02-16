#ifndef UNIXSERIALUSBANT_H
#define UNIXSERIALUSBANT_H

#include <QByteArray>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QStringList>

#include "antdevice.h"

namespace indoorcycling
{

class UnixSerialUsbAnt : public AntDevice
{
	Q_OBJECT
public:
	explicit UnixSerialUsbAnt(QObject *parent = 0);
	virtual ~UnixSerialUsbAnt();

	/** Find a Garmin USB1 Stick. If found, returns true, false otherwise */
	static bool isDevicePresent();

	virtual bool isValid() const;

	virtual int numberOfChannels() const;

	virtual int writeBytes(QByteArray& bytes);
	virtual QByteArray readBytes();
signals:
	
public slots:
private:
	static QFileInfoList findUsbSerialDevices();
	static bool isGarminUsb1Stick(const QFileInfo& fileInfo);

	int openConnection();

	QFileInfo _deviceFileInfo;
	int _nativeDeviceHandle;
};
}
#endif // UNIXSERIALUSBANT_H
