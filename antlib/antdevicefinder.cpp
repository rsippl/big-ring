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

#include "antdevicefinder.h"
#ifdef Q_OS_WIN
#include "usbexpressantdevice.h"
#else
#include "unixserialusbant.h"
#endif
#include "usb2antdevice.h"
#include <QtDebug>
extern "C" {
#include <libusb-1.0/libusb.h>
}
namespace indoorcycling {

AntDeviceFinder::AntDeviceFinder(QObject *parent) :
    QObject(parent)
{
    libusb_init(&_context);
}

AntDeviceFinder::~AntDeviceFinder()
{
    libusb_exit(_context);
}

AntDeviceType AntDeviceFinder::findAntDevice()
{
    AntDeviceType type = ANT_DEVICE_NONE;
    libusb_device** device_list;
    ssize_t nrOfDevices = libusb_get_device_list(_context, &device_list);
    for (int i = 0; i < nrOfDevices; ++i) {
        libusb_device* device = device_list[i];
        libusb_device_descriptor descriptor;
        libusb_get_device_descriptor(device, &descriptor);
        if (descriptor.idVendor == GARMIN_USB_VENDOR_ID) {
            if (descriptor.idProduct == GARMIN_USB2_PRODUCT_ID) {
                qDebug() << "found ANT+ USB2 device";
                type = ANT_DEVICE_USB_2;
            } else if (descriptor.idProduct == GARMIN_USB1_PRODUCT_ID) {
                qDebug() << "found ANT+ USB1 device";
                type = ANT_DEVICE_USB_1;
            }
        }
    }

    libusb_free_device_list(device_list, 1);
    return type;
}

QSharedPointer<AntDevice> AntDeviceFinder::openAntDevice()
{
    AntDeviceType type = findAntDevice();
    switch(type) {
    case ANT_DEVICE_USB_1:
#ifdef Q_OS_LINUX
        return QSharedPointer<AntDevice>(new UnixSerialUsbAnt);
#else
        return QSharedPointer<AntDevice>(new UsbExpressAntDevice);
#endif
    case ANT_DEVICE_USB_2:
        return QSharedPointer<AntDevice>(new Usb2AntDevice);
    default:
        return QSharedPointer<AntDevice>();
    }
}

}
