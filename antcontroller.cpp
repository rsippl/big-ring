#include "antcontroller.h"

#include <boost/shared_ptr.hpp>
#include "CommPort.h"

ANTController::ANTController(QObject *parent) :
	QObject(parent), ant(new ANT(this))
{
	initialize();

	connect(ant, SIGNAL(foundDevice(int,int,int)), SLOT(foundDevice(int,int,int)));
	connect(ant, SIGNAL(heartRate(quint8)), SIGNAL(heartRate(quint8)));
	connect(ant, SIGNAL(heartRate(quint8)), SLOT(receivedHeartRate(quint8)));
}

ANTController::~ANTController() {
	ant->stop();
	ant->wait();
}

void ANTController::foundDevice(int, int , int )
{
	qDebug() << "found ANT device";
//	ant->addDevice(device, device_type, channel);
}

void ANTController::initialize()
{
	QString errors;
	QVector<boost::shared_ptr<CommPort> > commPorts = CommPort::listCommPorts(errors);
	foreach(boost::shared_ptr<CommPort> commPort, commPorts) {
		QString deviceFile = commPort->name();
		if (ant->discover(deviceFile)) {
			qDebug() << "found device: " << deviceFile;
			ant->setDevice(deviceFile);
			ant->start();
		}
	}
}

void ANTController::receivedHeartRate(quint8 hr)
{
	qDebug() << "received hr: " << hr;
}
