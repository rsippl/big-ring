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
	void setCadence(float cadence);
	void setPower(float power);

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
