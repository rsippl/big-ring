QMAKE_CXXFLAGS += -std=c++11

TARGET = indoorcycling

DESTDIR = $$(PWD)
message(The project will be installed in $$DESTDIR)


CONFIG += DEBUG
QT += core concurrent gui opengl widgets
INCLUDEPATH += thirdparty/include

FORMS += rlvlistwidget.ui
RESOURCES += resources.qrc

HEADERS += \
    ANT.h \
    ANTChannel.h \
    ANTMessage.h \
    antcontroller.h \
    antdevice.h \
    antdevicefinder.h \
    antmessagegatherer.h \
    cyclist.h \
    mainwindow.h \
    profile.h \
    profilewidget.h \
    reallifevideoimporter.h \
    reallifevideo.h \
    rlvfileparser.h \
    rlvlistwidget.h \
    simulation.h \
    usb2antdevice.h \
    videocontroller.h \
    videodecoder.h \
    videowidget.h \
    thirdparty/include/GL/glew.h

SOURCES += \
    ANT.cpp \
    ANTChannel.cpp \
    ANTMessage.cpp \
    antcontroller.cpp \
    antdevice.cpp \
    antdevicefinder.cpp \
    antmessagegatherer.cpp \
    cyclist.cpp \
    main.cpp \
    mainwindow.cpp \
    profile.cpp \
    profilewidget.cpp \
    reallifevideoimporter.cpp \
    reallifevideo.cpp \
    rlvfileparser.cpp \
    rlvlistwidget.cpp \
    simulation.cpp \
    usb2antdevice.cpp \
    videocontroller.cpp \
    videodecoder.cpp \
    videowidget.cpp \
    thirdparty/src/glew.c

win32 {
    HEADERS += usbexpressantdevice.h
    SOURCES += usbexpressantdevice.cpp
}
!win32 {
    HEADERS += unixserialusbant.h
    SOURCES += unixserialusbant.cpp
}

LIBS +=  -lusb-1.0 -lavcodec -lavformat -lavutil




