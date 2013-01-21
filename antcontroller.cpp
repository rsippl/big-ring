#include "antcontroller.h"
#include "ANT.h"
#include <QMetaObject>

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

void ANTController::foundDevice(int, int , int , QString description, QString)
{
	qDebug() << "found ANT device";
	emit deviceFound(description);
}

void ANTController::initialize()
{
	antThread = new QThread(this);
	antThread->start();

	ant->moveToThread(antThread);

	connect(antTimer, SIGNAL(timeout()), ant, SLOT(readCycle()));
	connect(ant, SIGNAL(initializationSucceeded()), antTimer, SLOT(start()));
	connect(ant, SIGNAL(foundDevice(int,int,int,QString,QString)), SLOT(foundDevice(int,int,int,QString,QString)));
	connect(ant, SIGNAL(heartRateMeasured(quint8)), SIGNAL(heartRateMeasured(quint8)));
	connect(ant, SIGNAL(powerMeasured(float)), SIGNAL(powerMeasured(float)));
	connect(ant, SIGNAL(cadenceMeasured(float)), SIGNAL(cadenceMeasured(float)));

	QMetaObject::invokeMethod(ant, "initialize");
}

