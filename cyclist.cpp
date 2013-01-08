#include "cyclist.h"

namespace {
const float POWER = 300;
const float WEIGHT = 82.0f;
}
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

float Cyclist::power() const
{
	return POWER;
}

float Cyclist::weight() const
{
	return WEIGHT;
}
