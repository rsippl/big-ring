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

#ifndef USB2ANTDEVICE_H
#define USB2ANTDEVICE_H
#include "antdevice.h"

#include <memory>

namespace indoorcycling
{

struct Usb2DeviceConfiguration;

/**
 * @brief find a ANT+ Usb stick and report it's type
 * @return the type of USB ANT+ Stick, or ANT_DEVICE_NONE if none found.
 */
AntDeviceType findAntDeviceType();

/**
 * @brief class used for connecting to USB2 ANT+ devices. This implementation uses libusb-0.1
 */
class Usb2AntDevice : public AntDevice
{
    Q_OBJECT
public:
    explicit Usb2AntDevice(QObject *parent = 0);
    virtual ~Usb2AntDevice();
    virtual bool isValid() const;
    virtual int numberOfChannels() const;
    virtual int writeBytes(QByteArray& bytes);
    virtual QByteArray readBytes();
private:
    const std::unique_ptr<Usb2DeviceConfiguration> _deviceConfiguration;
};
}
#endif // USB2ANTDEVICE_H
