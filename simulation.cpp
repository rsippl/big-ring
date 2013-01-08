#include <QtDebug>
#include "simulation.h"
#include <math.h>
namespace {

/** Frontal area of a cyclist is around .5 m*m */
const float FRONTAL_AREA = 0.5f;
const float DRAG_COEFFICIENT = 0.5f;
const float AIR_DENSITY = 1.226f; // Sea level

/** Calculate drag from wind resistance */
float calculateAeroDrag(const Cyclist& cyclist)
{
	return FRONTAL_AREA * DRAG_COEFFICIENT * AIR_DENSITY * cyclist.speed() * cyclist.speed() * .5;
}

const float GRAVITY_CONSTANT = 9.81f;
const float ROLLING_RESISTANCE_COEFFICIENT = 0.004;

float calculateGroundResistance(const Cyclist& cylist)
{
	return cylist.weight() * GRAVITY_CONSTANT * ROLLING_RESISTANCE_COEFFICIENT;
}


float calculateGravityForce(const Cyclist& cyclist, float grade)
{
	return GRAVITY_CONSTANT * grade * 0.01 * cyclist.weight();
}

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

	_runTime = _runTime.addMSecs(elapsed);
	emit runTimeChanged(_runTime);
	float speed = calculateSpeed(elapsed);
	float distanceTravelled = (speed * elapsed) * 0.001;

	_cyclist.setSpeed(speed);
	_cyclist.setDistance(_cyclist.distance() + distanceTravelled);


	emit slopeChanged(_currentRlv.slopeForDistance(_cyclist.distance()));
	emit altitudeChanged(_currentRlv.altitudeForDistance(_cyclist.distance()));
}

void Simulation::rlvSelected(RealLiveVideo rlv)
{
	reset();
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

float Simulation::calculateSpeed(quint64 timeDelta)
{
	float force = (_cyclist.speed() > 0.5) ? _cyclist.power() / _cyclist.speed() : _cyclist.weight();

	float resistantForce = calculateAeroDrag(_cyclist) +
			calculateGravityForce(_cyclist, _currentRlv.slopeForDistance(_cyclist.distance())) +
			calculateGroundResistance(_cyclist);
	float resultingForce = force - resistantForce;

	float accelaration = resultingForce / _cyclist.weight();
	float speedChange = accelaration * timeDelta * 0.001;
	float speed = _cyclist.speed() + speedChange;
	return speed;
}

void Simulation::reset()
{
	play(false);
	_runTime = QTime();
	emit runTimeChanged(_runTime);
	_cyclist.setSpeed(0);
	_cyclist.setDistance(0);
	_cyclist.setDistanceTravelled(0);
}
