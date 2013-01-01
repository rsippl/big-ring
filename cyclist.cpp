#include "cyclist.h"

Cyclist::Cyclist(QObject *parent) :
	QObject(parent),  _speed(0), _distance(0), _distanceTravelled(0)
{
}

void Cyclist::setSpeed(float speed) {
	_speed = speed;
	emit speedChanged(speed);
}

void Cyclist::setDistance(float distance) {
	_distance = distance;
	emit distanceChanged(distance);
}

void Cyclist::setDistanceTravelled(float distanceTravelled)
{
	_distanceTravelled = distanceTravelled;
	emit distanceTravelledChanged(distanceTravelled);
}


float Cyclist::distance() const
{
	return _distance;
}

float Cyclist::distanceTravelled() const
{
	return _distanceTravelled;
}

float Cyclist::speed() const
{
	return _speed;
}
