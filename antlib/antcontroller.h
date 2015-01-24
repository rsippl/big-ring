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

#ifndef ANTCONTROLLER_H
#define ANTCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QTimer>


class ANT;

class ANTController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint8 heartRate READ heartRate NOTIFY heartRateMeasured)
    Q_PROPERTY(quint8 cadence READ cadence NOTIFY cadenceMeasured)
    Q_PROPERTY(quint16 power READ power NOTIFY powerMeasured)
public:
    explicit ANTController(QObject *parent = 0);
    virtual ~ANTController();

    bool isRunning() const;
    quint8 heartRate() const;
    quint16 power() const;
    quint8 cadence() const;
signals:
    void heartRateMeasured(quint8 bpm);
    void powerMeasured(quint16 power);
    void cadenceMeasured(quint8 cadence);

    void deviceFound(QString description);
    void finished();
public slots:
    void heartRateReceived(quint8 bpm);
    void powerReceived(float power);
    void cadenceReceived(float cadence);
    void quit();
private slots:
    void foundDevice(int channel, int device, int device_type, QString description, QString typeCode);
private:
    void initialize();

    quint8 _heartRate;
    quint16 _power;
    quint8 _cadence;

    QThread* antThread;
    QTimer* const antTimer;
};

#endif // ANTCONTROLLER_H
