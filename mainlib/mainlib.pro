TEMPLATE = lib

include(../config.pri)

RESOURCES += \
    $$IN_PWD/shaders.qrc \
    $$IN_PWD/flags.qrc

target = mainlib
target.path = ../lib
CONFIG += staticlib
INCLUDEPATH += thirdparty/include

FORMS +=

ANT_HEADERS += \
    ant/antdevice.h \
    ant/antdevicefinder.h \
    ant/antmessagegatherer.h \
    ant/usb2antdevice.h \
    ant/antmessage2.h \
    ant/antcentraldispatch.h \
    ant/antchannelhandler.h \
    ant/antsensortype.h \
    ant/antheartratechannelhandler.h \
    ant/antpowerchannelhandler.h \
    ant/antspeedandcadencechannelhandler.h \

ANT_SOURCES += \
    ant/antdevice.cpp \
    ant/antdevicefinder.cpp \
    ant/antmessagegatherer.cpp \
    ant/usb2antdevice.cpp \
    ant/antmessage2.cpp \
    ant/antcentraldispatch.cpp \
    ant/antchannelhandler.cpp \
    ant/antheartratechannelhandler.cpp \
    ant/antpowerchannelhandler.cpp \
    ant/antspeedandcadencechannelhandler.cpp

linux {
    ANT_SOURCES += ant/thirdparty/core.c
    ANT_HEADERS += ant/thirdparty/usb.h \
                   ant/thirdparty/usbi.h
}
!win32 {
    ANT_HEADERS += ant/unixserialusbant.h
    ANT_SOURCES += ant/unixserialusbant.cpp
}

HEADERS += \
    $$ANT_HEADERS \
    cyclist.h \
    framebuffer.h \
    profile.h \
    reallifevideoimporter.h \
    reallifevideo.h \
    rlvfileparser.h \
    simulation.h \
    thumbnailer.h \
    newvideowidget.h \
    clockgraphicsitem.h \
    sensoritem.h \
    profileitem.h \
    videoplayer.h \
    openglpainter2.h \
    run.h \
    screensaverblocker.h \
    profilepainter.h \
    quantityprinter.h \
    sensorconfiguration.h \
    sensors.h \
    virtualpower.h \
    bigringsettings.h \
    genericvideoreader.h \
    thumbnailcreatingvideoreader.h \
    framecopyingvideoreader.h \
    virtualtrainingfileparser.h \
    gpxfileparser.h \
    videoinforeader.h \
    utility.h \
    informationboxgraphicsitem.h \
    distancemappingentry.h \
    videoinformation.h \
    unitconverter.h \
    messagepanelitem.h \
    rollingaveragesensoritem.h \
    rollingaveragecalculator.h

SOURCES += \
    $$ANT_SOURCES \
    cyclist.cpp \
    profile.cpp \
    reallifevideoimporter.cpp \
    reallifevideo.cpp \
    rlvfileparser.cpp \
    simulation.cpp \
    thumbnailer.cpp \
    newvideowidget.cpp \
    clockgraphicsitem.cpp \
    sensoritem.cpp \
    profileitem.cpp \
    videoplayer.cpp \
    openglpainter2.cpp \
    run.cpp \
    screensaverblocker.cpp \
    profilepainter.cpp \
    quantityprinter.cpp \
    sensorconfiguration.cpp \
    sensors.cpp \
    virtualpower.cpp \
    bigringsettings.cpp \
    genericvideoreader.cpp \
    thumbnailcreatingvideoreader.cpp \
    framecopyingvideoreader.cpp \
    virtualtrainingfileparser.cpp \
    gpxfileparser.cpp \
    videoinforeader.cpp \
    informationboxgraphicsitem.cpp \
    distancemappingentry.cpp \
    videoinformation.cpp \
    unitconverter.cpp \
    messagepanelitem.cpp \
    rollingaveragesensoritem.cpp \
    rollingaveragecalculator.cpp

LIBS +=  -lusb-1.0

