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

const int GARMIN_USB_VENDOR_ID = 0x0fcf;
const int GARMIN_USB1_PRODUCT_ID = 0x1004;
const int GARMIN_USB2_PRODUCT_ID = 0x1008;


class AntDevice : public QObject
{
    Q_OBJECT
public:
    AntDevice(QObject* parent = 0);
    virtual ~AntDevice() {}
    virtual bool isValid() const = 0;
    virtual int numberOfChannels() const = 0;
    virtual int writeBytes(QByteArray& bytes) = 0;
    virtual QByteArray readBytes() = 0;
};
}
#endif // ANTDEVICE_H
