#-------------------------------------------------
#
# Project created by QtCreator 2014-10-26T17:17:44
#
#-------------------------------------------------
QT_VERSION = 5
QT       += core serialport
CONFIG += link_pkgconfig


TARGET = ../bin/anttestapp
TEMPLATE = app

include(../antlib/antlib.pri)


SOURCES += anttestapp.cpp

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

LIBS += $${ANTLIB_LIBRARY_DEPENDENCIES}
message("Master pro file path : ["$${ANTLIB_LIBRARY_DEPENCIES}"]")

