QT       -= gui

include(antlib.pri)

TARGET = antlib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    antdevice.cpp \
    antdevicefinder.cpp \
    antmessagegatherer.cpp \
    usb2antdevice.cpp \
    antmessage2.cpp \
    antcentraldispatch.cpp \
    antchannelhandler.cpp \
    antheartratechannelhandler.cpp \
    antpowerchannelhandler.cpp \
    antspeedandcadencechannelhandler.cpp

linux {
    SOURCES += core.c
    HEADERS += usb.h usbi.h
}

HEADERS += \
    antdevice.h \
    antdevicefinder.h \
    antmessagegatherer.h \
    usb2antdevice.h \
    antmessage2.h \
    antcentraldispatch.h \
    antchannelhandler.h \
    antsensortype.h \
    antheartratechannelhandler.h \
    antpowerchannelhandler.h \
    antspeedandcadencechannelhandler.h

win32 {
    HEADERS += usbexpressantdevice.h
    SOURCES += usbexpressantdevice.cpp
}
!win32 {
    HEADERS += unixserialusbant.h
    SOURCES += unixserialusbant.cpp
}
