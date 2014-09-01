QT       -= gui

include(antlib.pri)

TARGET = antlib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    ANT.cpp \
    ANTChannel.cpp \
    ANTMessage.cpp \
    antcontroller.cpp \
    antdevice.cpp \
    antdevicefinder.cpp \
    antmessagegatherer.cpp \
    usb2antdevice.cpp

HEADERS += \
    ANT.h \
    ANTChannel.h \
    ANTMessage.h \
    antcontroller.h \
    antdevice.h \
    antdevicefinder.h \
    antmessagegatherer.h \
    usb2antdevice.h

win32 {
    HEADERS += usbexpressantdevice.h
    SOURCES += usbexpressantdevice.cpp
}
!win32 {
    HEADERS += unixserialusbant.h
    SOURCES += unixserialusbant.cpp
}

