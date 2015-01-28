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

#ifndef ANTDEVICE_H
#define ANTDEVICE_H

#include <QByteArray>
#include <QObject>

namespace indoorcycling
{

enum AntDeviceType {
    ANT_DEVICE_NONE,
    ANT_DEVICE_USB_1,
    ANT_DEVICE_USB_2
};

/**
 * @brief abstract ANT device object. Used for reading and writing from and to ANT+ USB sticks.
 */
class AntDevice : public QObject
{
    Q_OBJECT
public:
    virtual ~AntDevice() = 0;
    /**
     * @brief Check if the device is connected correctly.
     * @return true if connected, false otherwise.
     */
    virtual bool isValid() const = 0;
    /**
     * @brief get the number of channels that the ANT+ device support.
     * @return the number of channels.
     */
    virtual int numberOfChannels() const = 0;
    /**
     * @brief write a byte array to the ANT+ device.
     * @param bytes the bytes to write.
     * @return the number of bytes written.
     */
    virtual int writeBytes(QByteArray& bytes) = 0;
    /**
     * @brief try to read a number of bytes from the ANT+ device.
     * @return the byte array that was read. Might be empty, in fact it *will* often be empty.
     */
    virtual QByteArray readBytes() = 0;
protected:
    AntDevice(QObject* parent = 0);
};

inline AntDevice::~AntDevice()
{
    // empty
}
}
#endif // ANTDEVICE_H
