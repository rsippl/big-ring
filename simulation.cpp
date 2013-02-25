#include <QtDebug>
#include "simulation.h"
#include <math.h>
namespace {

/** Frontal area of a cyclist is around .5 m*m */
const float FRONTAL_AREA = 0.58f;
const float DRAG_COEFFICIENT = 0.63f;
const float AIR_DENSITY = 1.226f; // Sea level

const float MINIMUM_SPEED = 0.5f; // m/s

const QTime MAX_IDLE_TIME(0, 0, 5);


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
	QObject(parent), _lastElapsed(0), _idleTime(),_cyclist(cyclist)
{
	_simulationUpdateTimer.setInterval(20);
	connect(&_simulationUpdateTimer, SIGNAL(timeout()), SLOT(simulationStep()));
}

Simulation::~Simulation()
{
	_simulationUpdateTimer.stop();
}

Cyclist &Simulation::cyclist() const
{
	return _cyclist;
}

void Simulation::play(bool play)
{
	if (play) {
		_simulationUpdateTimer.start();
		_simulationTime.restart();
		_lastElapsed = 0;
	} else {
		_simulationUpdateTimer.stop();
	}
	emit playing(play);
}

void Simulation::simulationStep()
{
	if (!_currentRlv.isValid())
		return;

	qint64 currentElapsed = _simulationTime.elapsed();
	qint64 elapsed = currentElapsed - _lastElapsed;
	_lastElapsed = currentElapsed;

	if (_cyclist.speed() > 0) {
		_runTime = _runTime.addMSecs(elapsed);
		emit runTimeChanged(_runTime);
	}

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
	if (_cyclist.power() < 1.0f) {
		// if there is no power input and current speed is zero, assume we have no input.
		if (_cyclist.speed() < MINIMUM_SPEED)
			return 0;

		// let the cyclist slow down and stop running after 3 seconds.
		_idleTime = _idleTime.addMSecs(timeDelta);

		if (_idleTime > MAX_IDLE_TIME)
			return 0;
	} else {
		_idleTime = QTime();
	}

	// if speed is very low, use cyclist weight, otherwise force gets very high. Is there
	// a better way to do this?
	float force = (_cyclist.speed() > (MINIMUM_SPEED - 0.1)) ? _cyclist.power() / _cyclist.speed() : _cyclist.weight();

	float resistantForce = calculateAeroDrag(_cyclist) +
			calculateGravityForce(_cyclist, _currentRlv.slopeForDistance(_cyclist.distance())) +
			calculateGroundResistance(_cyclist);
	float resultingForce = force - resistantForce;

	float accelaration = resultingForce / _cyclist.weight();
	float speedChange = accelaration * timeDelta * 0.001;
	float speed = _cyclist.speed() + speedChange;

	// If there's power applied, always return at least MINIMUM_SPEED.
	return qMax(MINIMUM_SPEED, speed);
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
