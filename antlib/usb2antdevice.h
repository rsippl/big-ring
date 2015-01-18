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

#include <QList>
#include <QTimer>

extern "C" {
#include <libusb-1.0/libusb.h>
}

namespace indoorcycling
{

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

private slots:
    void doTransfer();

private:
    /** callback functions */
    static void writeCallback(libusb_transfer* transfer);
    static void readCallback(libusb_transfer* transfer);
    void writeReady(libusb_transfer* transfer);
    void readReady(libusb_transfer* transfer);

    /** internal read and write functions */
    void doWrite();
    void doRead();

    libusb_context* _context;
    libusb_device_handle* _deviceHandle;
    libusb_transfer* _currentTransfer;

    QByteArray _writeBuffer;
    QByteArray _readBuffer;

    QTimer *_transferTimer;
    QByteArray _bytesRead;
    QList<QByteArray> _messagesToWrite;
    bool _wasAttached;
    bool _setupComplete;
};
}
#endif // USB2ANTDEVICE_H
