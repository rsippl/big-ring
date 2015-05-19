TEMPLATE = app
CONFIG += testcase
include(../config.pri)
include(../mainlib/mainlib.pri)
QT += testlib
TARGET = tests


SOURCES += \
    antmessage2test.cpp \
    main.cpp \
    virtualpowertest.cpp


HEADERS += \
    antmessage2test.h \
    common.h \
    virtualpowertest.h


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
