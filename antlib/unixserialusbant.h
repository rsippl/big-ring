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

#ifndef UNIXSERIALUSBANT_H
#define UNIXSERIALUSBANT_H

#include <QByteArray>
#include <QFileInfo>
#include <QFileInfoList>
#include <QObject>
#include <QStringList>

#include "antdevice.h"

namespace indoorcycling
{

/**
 * @brief ANT+ device that supports ANT+ USB1 Sticks. USB1 sticks use the CP210x USB to UART Bridge. These USB
 * sticks can be accessed using serial over USB communication. This is the Linux implementation.
 */
class UnixSerialUsbAnt : public AntDevice
{
    Q_OBJECT
public:
    explicit UnixSerialUsbAnt(QObject *parent = 0);
    virtual ~UnixSerialUsbAnt();

    virtual bool isValid() const;

    virtual int numberOfChannels() const;

    virtual int writeBytes(QByteArray& bytes);
    virtual QByteArray readBytes();
signals:

public slots:
private:
    static QFileInfoList findUsbSerialDevices();
    static bool isGarminUsb1Stick(const QFileInfo& fileInfo);

    int openConnection();

    QFileInfo _deviceFileInfo;
    int _nativeDeviceHandle;
};
}
#endif // UNIXSERIALUSBANT_H
