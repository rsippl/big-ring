#include "antcontroller.h"
#include <QMetaObject>
#include <boost/shared_ptr.hpp>
#include "CommPort.h"

ANTController::ANTController(QObject *parent) :
	QObject(parent), antThread(NULL), ant(new ANT),
	antTimer(new QTimer(this))
{
	initialize();
	antTimer->setInterval(50);
}

ANTController::~ANTController() {
	ant->deleteLater();
	antThread->quit();
	antThread->wait();
}

void ANTController::foundDevice(int, int , int )
{
	qDebug() << "found ANT device";
}

void ANTController::antInitialized()
{
	qDebug() << "ANT initialized";
	antTimer->start();
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
			createAntThread();
		}
	}
}

void ANTController::createAntThread()
{
	antThread = new QThread(this);
	antThread->start();

	ant->moveToThread(antThread);

	connect(ant, SIGNAL(initializationSucceeded()), SLOT(antInitialized()));
	connect(ant, SIGNAL(foundDevice(int,int,int)), SLOT(foundDevice(int,int,int)));
	connect(ant, SIGNAL(heartRate(quint8)), SIGNAL(heartRate(quint8)));

	connect(antTimer, SIGNAL(timeout()), ant, SLOT(readCycle()));

	QMetaObject::invokeMethod(ant, "initialize");
}

