QT       -= gui

include(antlib.pri)

TARGET = antlib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ANT.cpp \
    ANTChannel.cpp \
    antcontroller.cpp \
    antdevice.cpp \
    antdevicefinder.cpp \
    antmessagegatherer.cpp \
    usb2antdevice.cpp \
    antmessage2.cpp \
    antcentraldispatch.cpp

linux {
    SOURCES += core.c
    HEADERS += usb.h usbi.h
}

HEADERS += \
    ANT.h \
    ANTChannel.h \
    antcontroller.h \
    antdevice.h \
    antdevicefinder.h \
    antmessagegatherer.h \
    usb2antdevice.h \
    antmessage2.h \
    antcentraldispatch.h

win32 {
    HEADERS += usbexpressantdevice.h
    SOURCES += usbexpressantdevice.cpp
}
!win32 {
    HEADERS += unixserialusbant.h
    SOURCES += unixserialusbant.cpp
}

