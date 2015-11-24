TEMPLATE = lib

include(../config.pri)

RESOURCES += \
    $$IN_PWD/shaders.qrc

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
    model/distancemappingentry.h \
    model/profile.h \
    model/reallifevideo.h \
    model/rollingaveragecalculator.h \
    model/simulation.h \
    model/unitconverter.h \
    model/videoinformation.h \
    model/virtualpower.h

MODEL_SOURCES += \
    model/cyclist.cpp \
    model/distancemappingentry.cpp \
    model/profile.cpp \
    model/reallifevideo.cpp \
    model/rollingaveragecalculator.cpp \
    model/simulation.cpp \
    model/unitconverter.cpp \
    model/videoinformation.cpp \
    model/virtualpower.cpp

RIDEGUI_HEADERS += \
    ridegui/clockgraphicsitem.h \
    ridegui/informationboxgraphicsitem.h \
    ridegui/messagepanelitem.h \
    ridegui/newvideowidget.h \
    ridegui/profileitem.h \
    ridegui/rollingaveragesensoritem.h \
    ridegui/sensoritem.h

RIDEGUI_SOURCES += \
    ridegui/clockgraphicsitem.cpp \
    ridegui/informationboxgraphicsitem.cpp \
    ridegui/messagepanelitem.cpp \
    ridegui/newvideowidget.cpp \
    ridegui/profileitem.cpp \
    ridegui/rollingaveragesensoritem.cpp \
    ridegui/sensoritem.cpp



VIDEO_HEADERS += \
    video/framebuffer.h \
    video/genericvideoreader.h \
    video/openglpainter2.h \
    video/thumbnailcreatingvideoreader.h \
    video/framecopyingvideoreader.h \
    video/thumbnailer.h \
    video/videoinforeader.h \
    video/videoplayer.h

VIDEO_SOURCES += \
    video/genericvideoreader.cpp \
    video/openglpainter2.cpp \
    video/thumbnailcreatingvideoreader.cpp \
    video/framecopyingvideoreader.cpp \
    video/thumbnailer.cpp \
    video/videoinforeader.cpp \
    video/videoplayer.cpp


HEADERS += \
    $$ANT_HEADERS \
    $$FILEPARSER_HEADERS \
    $$MODEL_HEADERS \
    $$RIDEGUI_HEADERS \
    $$VIDEO_HEADERS \
    reallifevideoimporter.h \
    run.h \
    screensaverblocker.h \
    profilepainter.h \
    sensorconfiguration.h \
    sensors.h \
    bigringsettings.h \
    quantityprinter.h \
    util/util.h

SOURCES += \
    $$ANT_SOURCES \
    $$FILEPARSER_SOURCES \
    $$MODEL_SOURCES \
    $$RIDEGUI_SOURCES \
    $$VIDEO_SOURCES \
    reallifevideoimporter.cpp \
    run.cpp \
    screensaverblocker.cpp \
    profilepainter.cpp \
    quantityprinter.cpp \
    sensorconfiguration.cpp \
    sensors.cpp \
    bigringsettings.cpp

LIBS +=  -lusb-1.0

