#ifndef SIMULATION_H
#define SIMULATION_H

#include <QObject>
#include <QTime>
#include <QTimer>
#include "cyclist.h"
#include "reallifevideo.h"

class Simulation : public QObject
{
    Q_OBJECT
public:
    explicit Simulation(Cyclist& cyclist, QObject *parent = 0);
    virtual ~Simulation();

    Cyclist& cyclist() const;
    bool isPlaying();
signals:
    void slopeChanged(float slope);
    void altitudeChanged(float altitude);
    void runTimeChanged(QTime& runTime);

    void playing(bool playing);

public slots:
    void play(bool play);
    void simulationStep();
    void rlvSelected(RealLifeVideo rlv);
    void courseSelected(int courseNr);
    void courseSelected(const Course& course);


private:
    float calculateSpeed(quint64 timeDelta);
    void reset();

    qint64 _lastElapsed;
    QTime _runTime;
    QTime _simulationTime;
    QTime _idleTime;
    Cyclist& _cyclist;
    RealLifeVideo _currentRlv;
    QTimer _simulationUpdateTimer;
};

#endif // SIMULATION_H
