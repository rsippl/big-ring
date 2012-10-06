#ifndef ANTCONTROLLER_H
#define ANTCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>

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
	void antInitialized();
private:
	void initialize();
	void createAntThread();

	QThread* antThread;
	ANT* const ant;
	QTimer* const antTimer;
};

#endif // ANTCONTROLLER_H
