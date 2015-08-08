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

HEADERS += \
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
    gpxfileparser.h

SOURCES += \
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
    gpxfileparser.cpp



LIBS +=  -lusb-1.0




