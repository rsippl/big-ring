TEMPLATE = lib

include(mainlib.pri)

target = mainlib
target.path = ../lib
CONFIG += staticlib
INCLUDEPATH += thirdparty/include

FORMS += rlvlistwidget.ui

HEADERS += \
    cyclist.h \
    profile.h \
    reallifevideoimporter.h \
    reallifevideo.h \
    rlvfileparser.h \
    rlvlistwidget.h \
    simulation.h \
    thumbnailer.h \
    newvideowidget.h \
    clockgraphicsitem.h \
    sensoritem.h \
    profileitem.h \
    videoplayer.h \
    openglpainter.h \
    run.h

SOURCES += \
    cyclist.cpp \
    profile.cpp \
    reallifevideoimporter.cpp \
    reallifevideo.cpp \
    rlvfileparser.cpp \
    rlvlistwidget.cpp \
    simulation.cpp \
    thumbnailer.cpp \
    newvideowidget.cpp \
    clockgraphicsitem.cpp \
    sensoritem.cpp \
    profileitem.cpp \
    videoplayer.cpp \
    openglpainter.cpp \
    run.cpp



LIBS +=  -lusb-1.0




