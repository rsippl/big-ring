#include "antcontroller.h"
#include <QMetaObject>
#include <boost/shared_ptr.hpp>
#include "CommPort.h"

namespace {
const int TIMER_INTERVAL = 10; // ms
}
ANTController::ANTController(QObject *parent) :
	QObject(parent), antThread(NULL), ant(new ANT),
	antTimer(new QTimer(this))
{
	antTimer->setInterval(TIMER_INTERVAL);
	initialize();
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

void ANTController::initialize()
{
	antThread = new QThread(this);
	antThread->start();

	ant->moveToThread(antThread);

	connect(antTimer, SIGNAL(timeout()), ant, SLOT(readCycle()));
	connect(ant, SIGNAL(initializationSucceeded()), antTimer, SLOT(start()));
	connect(ant, SIGNAL(foundDevice(int,int,int)), SLOT(foundDevice(int,int,int)));
	connect(ant, SIGNAL(heartRate(quint8)), SIGNAL(heartRate(quint8)));

	QMetaObject::invokeMethod(ant, "initialize");
}

