#ifndef ANTCONTROLLER_H
#define ANTCONTROLLER_H

#include <QObject>

#include "ANT.h"

class ANTController : public QObject
{
	Q_OBJECT
public:
	explicit ANTController(QObject *parent = 0);
	virtual ~ANTController();
	
signals:
	void heartRate(quint8);
public slots:
	
private slots:
	void foundDevice(int channel, int device, int device_type);
	void receivedHeartRate(quint8 hr);
private:
	void initialize();


	ANT* const ant;
};

#endif // ANTCONTROLLER_H
