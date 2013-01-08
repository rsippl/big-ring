#ifndef SIMULATION_H
#define SIMULATION_H

#include <QDateTime>
#include <QObject>
#include <QTimer>
#include "cyclist.h"
#include "reallivevideo.h"

class Simulation : public QObject
{
	Q_OBJECT
public:
	explicit Simulation(Cyclist& cyclist, QObject *parent = 0);
	virtual ~Simulation();

	Cyclist& cyclist() const;
signals:
	void slopeChanged(float slope);
	void altitudeChanged(float altitude);

	void playing(bool playing);

public slots:
	void play(bool play);
	void simulationStep();
	void rlvSelected(RealLiveVideo rlv);
	void courseSelected(int courseNr);

private:
	float calculateSpeed(quint64 timeDelta);
	void reset();

	QDateTime _lastUpdateTime;
	Cyclist& _cyclist;
	RealLiveVideo _currentRlv;
	QTimer _simulationTimer;
};

#endif // SIMULATION_H
