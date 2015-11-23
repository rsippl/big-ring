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

FILEPARSER_HEADERS += \
    fileparsers/gpxfileparser.h \
    fileparsers/virtualtrainingfileparser.h \
    fileparsers/rlvfileparser.h

FILEPARSER_SOURCES += \
    fileparsers/gpxfileparser.cpp \
    fileparsers/virtualtrainingfileparser.cpp \
    fileparsers/rlvfileparser.cpp


MODEL_HEADERS += \
    model/cyclist.h \
    model/profile.h \
    model/reallifevideo.h \
    model/rollingaveragecalculator.h \
    model/simulation.h \
    model/unitconverter.h \
    model/virtualpower.h

MODEL_SOURCES += \
    model/cyclist.cpp \
    model/profile.cpp \
    model/reallifevideo.cpp \
    model/rollingaveragecalculator.cpp \
    model/simulation.cpp \
    model/unitconverter.cpp \
    model/virtualpower.cpp

HEADERS += \
    $$ANT_HEADERS \
    $$FILEPARSER_HEADERS \
    $$MODEL_HEADERS \
    framebuffer.h \
    reallifevideoimporter.h \
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
    sensorconfiguration.h \
    sensors.h \
    bigringsettings.h \
    genericvideoreader.h \
    thumbnailcreatingvideoreader.h \
    framecopyingvideoreader.h \
    videoinforeader.h \
    utility.h \
    quantityprinter.h \
    informationboxgraphicsitem.h \
    distancemappingentry.h \
    videoinformation.h \
    messagepanelitem.h \
    rollingaveragesensoritem.h

SOURCES += \
    $$ANT_SOURCES \
    $$FILEPARSER_SOURCES \
    $$MODEL_SOURCES \
    reallifevideoimporter.cpp \
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
    bigringsettings.cpp \
    genericvideoreader.cpp \
    thumbnailcreatingvideoreader.cpp \
    framecopyingvideoreader.cpp \
    videoinforeader.cpp \
    informationboxgraphicsitem.cpp \
    distancemappingentry.cpp \
    videoinformation.cpp \
    messagepanelitem.cpp \
    rollingaveragesensoritem.cpp \

LIBS +=  -lusb-1.0

