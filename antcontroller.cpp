#include "antcontroller.h"
#include "ANT.h"
#include <QCoreApplication>

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
	/* busy waiting for antThread to shutdown. */
	while(antThread->isRunning())
		QCoreApplication::processEvents();
}

void ANTController::foundDevice(int, int , int , QString description, QString)
{
	qDebug() << "found ANT device";
	emit deviceFound(description);
}

void ANTController::initialize()
{
	antThread = new QThread(this);


	ant->moveToThread(antThread);

	connect(antThread, SIGNAL(started()), ant, SLOT(initialize()));
	connect(ant, SIGNAL(destroyed()), antThread, SLOT(quit()));

	connect(antTimer, SIGNAL(timeout()), ant, SLOT(readCycle()));
	connect(ant, SIGNAL(initializationSucceeded()), antTimer, SLOT(start()));
	connect(ant, SIGNAL(foundDevice(int,int,int,QString,QString)), SLOT(foundDevice(int,int,int,QString,QString)));
	connect(ant, SIGNAL(heartRateMeasured(quint8)), SIGNAL(heartRateMeasured(quint8)));
	connect(ant, SIGNAL(powerMeasured(float)), SIGNAL(powerMeasured(float)));
	connect(ant, SIGNAL(cadenceMeasured(float)), SIGNAL(cadenceMeasured(float)));

	antThread->start();
}

