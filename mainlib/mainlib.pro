TEMPLATE = lib

include(mainlib.pri)

target = mainlib
target.path = ../lib
CONFIG += staticlib
INCLUDEPATH += thirdparty/include

FORMS += rlvlistwidget.ui

HEADERS += \
    cyclist.h \
    mainwindow.h \
    profile.h \
    profilewidget.h \
    reallifevideoimporter.h \
    reallifevideo.h \
    rlvfileparser.h \
    rlvlistwidget.h \
    simulation.h \
    videocontroller.h \
    videodecoder.h \
    videowidget.h \
    thirdparty/include/GL/glew.h

SOURCES += \
    cyclist.cpp \
    mainwindow.cpp \
    profile.cpp \
    profilewidget.cpp \
    reallifevideoimporter.cpp \
    reallifevideo.cpp \
    rlvfileparser.cpp \
    rlvlistwidget.cpp \
    simulation.cpp \
    videocontroller.cpp \
    videodecoder.cpp \
    videowidget.cpp \
    thirdparty/src/glew.c



LIBS +=  -lusb-1.0 -lavcodec -lavformat -lavutil




