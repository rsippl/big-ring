#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

class Usb2AntDevice : public AntDevice
{
	Q_OBJECT
public:
	explicit Usb2AntDevice(QObject *parent = 0);
	virtual ~Usb2AntDevice();
	virtual bool isValid() const;
	virtual int numberOfChannels() const;
	virtual int writeBytes(QByteArray& bytes);
	virtual QByteArray readBytes();
signals:
	
public slots:
	
};

#endif // USB2ANTDEVICE_H
