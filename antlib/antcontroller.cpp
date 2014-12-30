#include "antcontroller.h"
#include "ANT.h"
#include <QCoreApplication>

namespace {
const int TIMER_INTERVAL = 10; // ms
}
ANTController::ANTController(QObject *parent) :
    QObject(parent), _heartRate(0), _power(0), _cadence(0), antThread(new QThread(this)),
    antTimer(new QTimer(this))
{
	antTimer->setInterval(TIMER_INTERVAL);
	initialize();
}

ANTController::~ANTController() {
    if (antThread->isRunning()) {
        qWarning("Call quit() before destroying ANTController");
    }
}

quint8 ANTController::heartRate() const
{
	return _heartRate;
}

quint16 ANTController::power() const
{
	return _power;
}

quint8 ANTController::cadence() const
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
    ANT* ant = new ANT;
	ant->moveToThread(antThread);

	connect(antThread, SIGNAL(started()), ant, SLOT(initialize()));
    connect(antThread, SIGNAL(finished()), ant, SLOT(deleteLater()));

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
	_cadence = static_cast<quint8>(cadence);
    emit cadenceMeasured(_cadence);
}

void ANTController::quit()
{
    antThread->quit();
}

void ANTController::powerReceived(float power)
{
	_power = static_cast<quint16>(power);
	emit powerMeasured(_power);
}

