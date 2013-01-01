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

signals:
	void speedChanged(float speed);
	/** distance on track changed */
	void distanceChanged(float distance);
	/** distance travelled from start of course */
	void distanceTravelledChanged(float distanceTravelled);

private:
	float _speed;
	float _distance;
	float _distanceTravelled;
};

#endif // CYCLIST_H
