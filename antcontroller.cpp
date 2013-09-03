#include "antcontroller.h"
#include "ANT.h"
#include <QCoreApplication>

namespace {
const int TIMER_INTERVAL = 10; // ms
}
ANTController::ANTController(QObject *parent) :
	QObject(parent), _heartRate(0), _power(0), _cadence(0), antThread(NULL), ant(new ANT),
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

quint8 ANTController::heartRate() const
{
	return _heartRate;
}

float ANTController::power() const
{
	return _power;
}

float ANTController::cadence() const
{
	return _cadence;
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
	connect(ant, SIGNAL(heartRateMeasured(quint8)), SLOT(heartRateReceived(quint8)));
	connect(ant, SIGNAL(powerMeasured(float)), SLOT(powerReceived(float)));
	connect(ant, SIGNAL(cadenceMeasured(float)), SLOT(cadenceReceived(float)));

	antThread->start();
}


void ANTController::heartRateReceived(quint8 bpm)
{
	_heartRate = bpm;
	emit heartRateMeasured(bpm);
}

void ANTController::cadenceReceived(float cadence)
{
	_cadence = cadence;
	emit cadenceMeasured(cadence);
}

void ANTController::powerReceived(float power)
{
	_power = power;
	emit powerMeasured(power);
}

