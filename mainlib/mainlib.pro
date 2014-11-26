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
    profilewidget.h \
    reallifevideoimporter.h \
    reallifevideo.h \
    rlvfileparser.h \
    rlvlistwidget.h \
    simulation.h \
    newvideowidget.h \
    clockgraphicsitem.h

SOURCES += \
    cyclist.cpp \
    profile.cpp \
    profilewidget.cpp \
    reallifevideoimporter.cpp \
    reallifevideo.cpp \
    rlvfileparser.cpp \
    rlvlistwidget.cpp \
    simulation.cpp \
    newvideowidget.cpp \
    clockgraphicsitem.cpp



LIBS +=  -lusb-1.0




