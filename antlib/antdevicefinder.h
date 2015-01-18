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

#ifndef ANTDEVICEFINDER_H
#define ANTDEVICEFINDER_H

#include <QObject>
#include <QSharedPointer>
#include "antdevice.h"
struct libusb_context;

namespace indoorcycling {

class AntDeviceFinder : public QObject
{
    Q_OBJECT
public:
    explicit AntDeviceFinder(QObject *parent = 0);
    virtual ~AntDeviceFinder();

    AntDeviceType findAntDevice();

    /** Open an AntDevice. Returns an invalid pointer if no device can be found. */
    QSharedPointer<AntDevice> openAntDevice();
private:
    libusb_context* _context;
};
}
#endif // ANTDEVICEFINDER_H
