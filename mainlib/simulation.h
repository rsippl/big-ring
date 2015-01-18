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
