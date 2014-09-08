TEMPLATE = app
CONFIG += testcase
include(../indoorcycling.pri)
include(../mainlib/mainlib.pri)
QT += testlib
TARGET = tests

SOURCES += \
	antmessagegatherertest.cpp \
    reallifevideotest.cpp \
    testmain.cpp


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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../google_test/release/ -lgoogle_test
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../google_test/debug/ -lgoogle_test
else:unix: LIBS += -L$$OUT_PWD/../google_test/ -lgoogle_test

INCLUDEPATH += $$PWD/../google_test
DEPENDPATH += $$PWD/../google_test

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../google_test/release/libgoogle_test.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../google_test/debug/libgoogle_test.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../google_test/release/google_test.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../google_test/debug/google_test.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../google_test/libgoogle_test.a
