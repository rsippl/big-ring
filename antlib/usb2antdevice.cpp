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

#include "usb2antdevice.h"
#include <QtDebug>
#include <QtCore/QThread>
namespace
{
bool usbInitialized = false;
}

namespace indoorcycling
{

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
    AntDevice(parent), _deviceHandle(nullptr)
{
    _deviceHandle = openAntStick();
    usb_clear_halt(_deviceHandle, _writeEndpoint);
    usb_clear_halt(_deviceHandle, _readEndpoint);
}

Usb2AntDevice::~Usb2AntDevice() {
    if (_deviceHandle) {
        usb_release_interface(_deviceHandle, _interface);
        usb_close(_deviceHandle);
    }
}

bool Usb2AntDevice::isValid() const
{
    return (_deviceHandle);
}

int Usb2AntDevice::numberOfChannels() const
{
    return 8;
}

int Usb2AntDevice::writeBytes(QByteArray &bytes)
{
#ifdef Q_OS_WIN
    return usb_interrupt_write(_deviceHandle, _writeEndpoint, bytes.data(), bytes.size(), 50);
#else
    int rc = usb_bulk_write(_deviceHandle, _writeEndpoint, bytes.data(), bytes.size(), 50);
    if (rc < 0) {
        qWarning("usb error: %s", usb_strerror());
    }
    return rc;
#endif
}

/**
 * @brief read all available bytes from the ANT+ stick.
 * @return a byte array with all the bytes that were read. Can be empty.
 */
QByteArray Usb2AntDevice::readBytes()
{
    QByteArray bytesRead;
    bool bytesAvailable = true;
    while(bytesAvailable) {
        QByteArray buffer(128, 0);

        int nrOfBytesRead = usb_bulk_read(_deviceHandle, _readEndpoint, buffer.data(), buffer.size(), 10);
        if (nrOfBytesRead < 0) {
            bytesAvailable = false;
        } else {
            bytesRead.append(buffer.left(nrOfBytesRead));
            bytesAvailable = (nrOfBytesRead == buffer.size());
        }
    }
    return bytesRead;
}

void Usb2AntDevice::initializeUsb()
{
    if (!usbInitialized) {
        usb_set_debug(255);
        usb_init();

        usb_find_busses();
        usb_find_devices();
        usbInitialized = true;
    }
}

struct usb_device *Usb2AntDevice::findAntStick()
{
    initializeUsb();
    struct usb_bus* bus;
    struct usb_device* device;

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (device = bus->devices; device; device = device->next) {
            if (device->descriptor.idVendor == GARMIN_USB_VENDOR_ID &&
                    (device->descriptor.idProduct == GARMIN_USB2_PRODUCT_ID || device->descriptor.idProduct == OEM_USB2_PRODUCT_ID)) {
                return device;
            }
        }
    }
    return nullptr;
}

void Usb2AntDevice::resetAntStick(struct usb_device *antStick)
{
    struct usb_dev_handle* antStickHandle;
    if ((antStickHandle = usb_open(antStick))) {
        usb_reset(antStickHandle);
        usb_close(antStickHandle);
    } else {
        qWarning("Unable to open and reset ANT stick");
    }
}

usb_dev_handle *Usb2AntDevice::openAntStick()
{
    struct usb_dev_handle* deviceHandle;

    struct usb_device* device = findAntStick();
    if (device) {
        resetAntStick(device);

        deviceHandle = usb_open(device);
        if (deviceHandle && device->descriptor.bNumConfigurations) {
            if ((_intf = findUsbInterface(&device->config[0])) != NULL) {
                qDebug() << deviceHandle;
                int rc;
#ifdef Q_OS_LINUX
                rc = usb_detach_kernel_driver_np(deviceHandle, _interface);
                if (rc < 0) {
                    qDebug() << "usb error" << usb_strerror();
                }
#endif
                rc = usb_set_configuration(deviceHandle, 1);
                if (rc < 0) {
                    qDebug()<<"usb_set_configuration Error: "<< usb_strerror();
                }

                rc = usb_claim_interface(deviceHandle, _interface);
                if (rc < 0) qDebug()<<"usb_claim_interface Error: "<< usb_strerror();
                return deviceHandle;
            }
        }
    } else {
        qWarning("Unable to find an ANT+ USB2 stick to open");
    }

    return nullptr;
}

usb_interface_descriptor *Usb2AntDevice::findUsbInterface(usb_config_descriptor *config_descriptor)
{
    struct usb_interface_descriptor* intf;

    _readEndpoint = -1;
    _writeEndpoint = -1;
    _interface = -1;

    if (!config_descriptor) {
        qDebug() << "config descriptor is null";
        return nullptr;
    }

    qDebug() << "num interface" << config_descriptor->bNumInterfaces;
    if (!config_descriptor->bNumInterfaces) {
        return nullptr;
    }
    qDebug() << "num alt_setting" << config_descriptor->interface[0].num_altsetting;
    if (!config_descriptor->interface[0].num_altsetting) return nullptr;

    intf = &config_descriptor->interface[0].altsetting[0];

    qDebug() << "num endpoints" << intf->bNumEndpoints;


    if (intf->bNumEndpoints != 2) return nullptr;


    qDebug() << "interface number" << intf->bInterfaceNumber;

    _interface = intf->bInterfaceNumber;

    for (int i = 0 ; i < 2; i++)
    {
        if (intf->endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
            _readEndpoint = intf->endpoint[i].bEndpointAddress;
        else
            _writeEndpoint = intf->endpoint[i].bEndpointAddress;
    }

    qDebug() << "read end point" << _readEndpoint;
    qDebug() << "write end point" << _writeEndpoint;

    if (_readEndpoint < 0 || _writeEndpoint < 0)
        return nullptr;

    return intf;
}
}

