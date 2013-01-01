#include "simulation.h"

namespace {
const float SPEED = 25.0f / 3.6f;
}

Simulation::Simulation(Cyclist &cyclist, QObject *parent) :
	QObject(parent), _cyclist(cyclist)
{
	_simulationTimer.setInterval(20);
	connect(&_simulationTimer, SIGNAL(timeout()), SLOT(simulationStep()));
}

Simulation::~Simulation()
{
	_simulationTimer.stop();
}

Cyclist &Simulation::cyclist() const
{
	return _cyclist;
}



void Simulation::play(bool play)
{
	if (play) {
		_lastUpdateTime = QDateTime::currentDateTime();
		_simulationTimer.start();
	} else {
		_simulationTimer.stop();
	}
	emit playing(play);
}

void Simulation::simulationStep()
{
//	qDebug() << "simulation step";
	if (!_currentRlv.isValid())
		return;

	QDateTime currentTime = QDateTime::currentDateTime();
	qint64 elapsed = _lastUpdateTime.msecsTo(currentTime);
	_lastUpdateTime = currentTime;

	float distanceTravelled = (SPEED * elapsed) * 0.001;

	_cyclist.setSpeed(SPEED);
	_cyclist.setDistance(_cyclist.distance() + distanceTravelled);


	emit slopeChanged(_currentRlv.slopeForDistance(_cyclist.distance()));
	emit altitudeChanged(_currentRlv.altitudeForDistance(_cyclist.distance()));
}

void Simulation::rlvSelected(RealLiveVideo rlv)
{
	_currentRlv = rlv;
}

void Simulation::courseSelected(int courseNr)
{
	reset();
	if (courseNr == -1) {
		return;
	}
	if (!_currentRlv.isValid())
		return;

	const Course& course = _currentRlv.courses()[courseNr];
	_cyclist.setDistance(course.start());
}

void Simulation::reset()
{
	_cyclist.setSpeed(0);
	_cyclist.setDistance(0);
	_cyclist.setDistanceTravelled(0);
}

