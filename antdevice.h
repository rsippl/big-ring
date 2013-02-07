#ifndef ANTDEVICE_H
#define ANTDEVICE_H

#include <QByteArray>
#include <QObject>

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

#endif // ANTDEVICE_H
