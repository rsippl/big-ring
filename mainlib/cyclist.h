/*
 * Copyright (c) 2012-2015 Ilja Booij (ibooij@gmail.com)
 *
 * This file is part of Big Ring Indoor Video Cycling
 *
 * Big Ring Indoor Video Cycling is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 *
 * Big Ring Indoor Video Cycling  is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Big Ring Indoor Video Cycling.  If not, see 
 * <http://www.gnu.org/licenses/>.
 */

#ifndef CYCLIST_H
#define CYCLIST_H

#include <QObject>

class Cyclist : public QObject
{
	Q_OBJECT
public:
	explicit Cyclist(QObject *parent = 0);
	void setDistance(float distance);
	void setDistanceTravelled(float distanceTravelled);
	void setSpeed(float speed);

	float distance() const;
	float distanceTravelled() const;
	float speed() const;
	float heartRate() const;
	float cadence() const;
	float power() const;
	float weight() const;

public slots:
	void setHeartRate(quint8 heartRate);
	void setCadence(quint8 cadence);
	void setPower(quint16 power);

signals:
	void heartRateChanged(quint8 heartRate);
	void cadenceChanged(float cadence);
	void powerChanged(float power);

	void speedChanged(float speed);
	/** distance on track changed */
	void distanceChanged(float distance);
	/** distance travelled from start of course */
	void distanceTravelledChanged(float distanceTravelled);

private:
	quint8 _heartRate;
	float _cadence;
	float _power;

	float _speed;
	float _distance;
	float _distanceTravelled;
};

#endif // CYCLIST_H
