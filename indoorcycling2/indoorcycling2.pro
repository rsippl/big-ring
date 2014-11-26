#-------------------------------------------------
#
# Project created by QtCreator 2014-10-26T17:17:44
#
#-------------------------------------------------
QT_VERSION = 5
QT       += core gui


TARGET = indoorcycling2
TEMPLATE = app

INCLUDEPATH += /home/ibooij/local/lib/qtgstreamer/include/Qt5GStreamer


include(../mainlib/mainlib.pri)


SOURCES += main.cpp\
        mainwindow.cpp \
    rlvtablemodel.cpp \
    previewvideowidget.cpp \
    clockitem.cpp

HEADERS  += mainwindow.h \
    rlvtablemodel.h \
    previewvideowidget.h \
    clockitem.h

LIBS += -L/home/ibooij/local/lib/qtgstreamer/lib/x86_64-linux-gnu -lQt5GLib-2.0 -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0

# added by QtCreator to add mainlib to dependencies.
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../mainlib/release/ -lmainlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../mainlib/debug/ -lmainlib
else:unix: LIBS += -L$$OUT_PWD/../mainlib/ -lmainlib

INCLUDEPATH += $$PWD/../mainlib
DEPENDPATH += $$PWD/../mainlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/release/libmainlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/debug/libmainlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/release/mainlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../mainlib/debug/mainlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../mainlib/libmainlib.a

# added by QtCreator to add antlib to dependencies.
win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../antlib/release/ -lantlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../antlib/debug/ -lantlib
else:unix: LIBS += -L$$OUT_PWD/../antlib/ -lantlib

INCLUDEPATH += $$PWD/../antlib
DEPENDPATH += $$PWD/../antlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../antlib/release/libantlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../antlib/debug/libantlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../antlib/release/antlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../antlib/debug/antlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../antlib/libantlib.a
message("Master pro file path : ["$${MAINLIB_LIBRARY_DEPENCIES}"]")
LIBS += $${MAINLIB_LIBRARY_DEPENCIES}

FORMS += \
    mainwindow.ui

