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

extern "C" {
#include <usb.h>
}

namespace
{

const int GARMIN_USB_VENDOR_ID = 0x0fcf;
const int GARMIN_USB1_PRODUCT_ID = 0x1004;
const int GARMIN_USB2_PRODUCT_ID = 0x1008;
const int OEM_USB2_PRODUCT_ID = 0x1009;

bool usbInitialized = false;

/**
 * @brief initialize the usb library. This function can be called multiple times, but will only "work" once.
 */
void initializeUsb();
/**
 * @brief find an ANT+ Stick on the USB ports.
 * @return the usb_device found, or nullptr if nothing found.
 */
static struct usb_device* findAntStick();
/**
 * @brief reset the ant stick. We'll use this before connecting.
 * @param antStick
 */
void resetAntStick(struct usb_device* antStick);
/**
 * @brief open a connection to the ANT+ stick and initialize communication interfaces.
 * @return the full USB configuration.
 */
std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> openAntStick();
/**
 * @brief initialize communication interfaces for the ant stick.
 * @return the USB configuration.
 */
std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> findUsbInterface(usb_config_descriptor &config_descriptor);

#ifdef Q_OS_LINUX
/** On Linux, we might have to detach the kernel driver. */
int detachKernelDriver(struct usb_dev_handle& usbDeviceHandle, int interface);
#endif
}

namespace indoorcycling
{

AntDeviceType findAntDeviceType()
{
    struct usb_device* device = findAntStick();
    if (device) {
        switch(device->descriptor.idProduct) {
        case GARMIN_USB1_PRODUCT_ID:
            return ANT_DEVICE_USB_1;
        case GARMIN_USB2_PRODUCT_ID:
        case OEM_USB2_PRODUCT_ID:
            return ANT_DEVICE_USB_2;
        }
    }
    return ANT_DEVICE_NONE;
}

/**
 * @brief struct containing the usb configuration that we'll use for reading and writing.
 */
struct Usb2DeviceConfiguration
{
    struct usb_dev_handle* deviceHandle;
    int readEndpoint;
    int writeEndpoint;
    int interface;
};

Usb2AntDevice::Usb2AntDevice(QObject *parent) :
    AntDevice(parent), _deviceConfiguration(openAntStick())
{
    if (!_deviceConfiguration) {
        qWarning("Unable to open ANT+ stick correctly");
    }
    qDebug() << "trying to empty the ANT stick's buffer";
    QByteArray buffer = readBytes();
    if (!buffer.isEmpty()) {
        qDebug() << "buffer was not empty";
    }
}

Usb2AntDevice::~Usb2AntDevice() {
    if (_deviceConfiguration) {
        usb_release_interface(_deviceConfiguration->deviceHandle, _deviceConfiguration->interface);
        usb_close(_deviceConfiguration->deviceHandle);
    }
}

bool Usb2AntDevice::isValid() const
{
    return (_deviceConfiguration.get() != nullptr);
}

int Usb2AntDevice::numberOfChannels() const
{
    return 8;
}

int Usb2AntDevice::writeBytes(QByteArray &bytes)
{
    if (!_deviceConfiguration) {
        qWarning("Trying to read without a connection to a USB device");
        return 0;
    }
#ifdef Q_OS_WIN
    return usb_interrupt_write(_deviceConfiguration->deviceHandle, _deviceConfiguration->writeEndpoint, bytes.data(), bytes.size(), 50);
#else
    qDebug() << "writing " << bytes.size() << "bytes";
    int rc = usb_bulk_write(_deviceConfiguration->deviceHandle, _deviceConfiguration->writeEndpoint, bytes.data(), bytes.size(), 50);
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
    if (!_deviceConfiguration) {
        qWarning("Trying to write without a connection to a USB device");
        return 0;
    }
    QByteArray bytesRead;
    bool bytesAvailable = true;
    int loopNr = 0;
    while(bytesAvailable) {
        if (loopNr > 0) {
            qDebug() << "loopNr" << loopNr;
        }
        QByteArray buffer(64, 0);

        int nrOfBytesRead = usb_bulk_read(_deviceConfiguration->deviceHandle, _deviceConfiguration->readEndpoint, buffer.data(), buffer.size(), 50);
        if (nrOfBytesRead <= 0) {
            if (nrOfBytesRead != -ETIMEDOUT) {
                qDebug() << "usb returns" << nrOfBytesRead << usb_strerror();
            }
            bytesAvailable = false;
        } else {
            bytesRead.append(buffer.left(nrOfBytesRead));
            bytesAvailable = (nrOfBytesRead == buffer.size());
            buffer.clear();
        }
        loopNr += 1;
    }
    return bytesRead;
}

} // end namespace indoorcycling

namespace
{

void initializeUsb()
{
    if (!usbInitialized) {
        usb_set_debug(255);
        usb_init();

        usb_find_busses();
        usb_find_devices();
        usbInitialized = true;
    }
}

struct usb_device *findAntStick()
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

void resetAntStick(struct usb_device *antStick)
{
    struct usb_dev_handle* antStickHandle;
    if ((antStickHandle = usb_open(antStick))) {
        usb_reset(antStickHandle);
        usb_close(antStickHandle);
    } else {
        qWarning("Unable to open and reset ANT stick");
    }
}

std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> openAntStick()
{
    struct usb_dev_handle* deviceHandle;

    struct usb_device* device = findAntStick();
    if (device) {
        resetAntStick(device);

        deviceHandle = usb_open(device);
        if (deviceHandle && device->descriptor.bNumConfigurations) {
            std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> deviceConfiguration = findUsbInterface(*(&device->config[0]));
            if (deviceConfiguration) {
                deviceConfiguration->deviceHandle = deviceHandle;
                int rc;
#ifdef Q_OS_LINUX
                detachKernelDriver(*deviceHandle, deviceConfiguration->interface);
#endif
                rc = usb_set_configuration(deviceHandle, 1);
                if (rc < 0) {
                    qDebug()<<"usb_set_configuration Error: "<< usb_strerror();
                }

                rc = usb_claim_interface(deviceHandle, deviceConfiguration->interface);
                if (rc < 0) qDebug()<<"usb_claim_interface Error: "<< usb_strerror();

                usb_clear_halt(deviceHandle, deviceConfiguration->writeEndpoint);
                usb_clear_halt(deviceHandle, deviceConfiguration->readEndpoint);

                qDebug() << "USB2 Ant Stick connected";
                return deviceConfiguration;
            }
        }
    } else {
        qWarning("Unable to find an ANT+ USB2 stick to open");
    }

    return nullptr;
}

std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> findUsbInterface(usb_config_descriptor& config_descriptor)
{
    if (!config_descriptor.bNumInterfaces) {
        qWarning("No interfaces found on USB device");
        return nullptr;
    }
    if (!config_descriptor.interface[0].num_altsetting) {
        qWarning("No alternative interfaces found on USB device");
        return nullptr;
    }

    std::unique_ptr<indoorcycling::Usb2DeviceConfiguration> deviceConfiguration(new indoorcycling::Usb2DeviceConfiguration);

    struct usb_interface_descriptor intf = config_descriptor.interface[0].altsetting[0];

    if (intf.bNumEndpoints != 2) {
        qWarning("The USB device needs to have a read and a write endpoint");
        return nullptr;
    }
    deviceConfiguration->interface = intf.bInterfaceNumber;

    for (int i = 0 ; i < 2; i++) {
        if (intf.endpoint[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK)
            deviceConfiguration->readEndpoint = intf.endpoint[i].bEndpointAddress;
        else
            deviceConfiguration->writeEndpoint = intf.endpoint[i].bEndpointAddress;
    }

    if (deviceConfiguration->readEndpoint < 0) {
        qWarning("USB device read end point not found.");
        return nullptr;
    }
    if (deviceConfiguration->writeEndpoint < 0) {
        qWarning("USB device write end point not found.");
        return nullptr;
    }

    return deviceConfiguration;
}

#ifdef Q_OS_LINUX
int detachKernelDriver(usb_dev_handle &usbDeviceHandle, int interface)
{
    QByteArray driverNameBuffer(128, '0');
    if (usb_get_driver_np(&usbDeviceHandle, interface, driverNameBuffer.data(), driverNameBuffer.size()) >= 0) {
        qDebug() << "We need to detach the kernel driver with name:" << QString(driverNameBuffer);
        if (usb_detach_kernel_driver_np(&usbDeviceHandle, interface) < 0) {
            qWarning("Unable to detach kernel driver for usb ANT+ stick: %s", usb_strerror());
            return -1;
        } else {
            qDebug() << "Kernel driver detached, we can now connect";
        }
    } else {
        qDebug() << "There is no kernel driver attached to the ANT+ Stick. We can connect to it.";
    }
    return 0;
}
#endif

}

