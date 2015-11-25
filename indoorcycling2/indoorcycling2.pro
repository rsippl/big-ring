#-------------------------------------------------
#
# Project created by QtCreator 2014-10-26T17:17:44
#
#-------------------------------------------------

TARGET = ../bin/big-ring
TEMPLATE = app

win32:VERSION = 1.2.0.0
linux:VERSION = 1.2.0.0

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

include(../config.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    videolistview.cpp \
    videolistmodel.cpp \
    videoitemdelegate.cpp \
    settingsdialog.cpp \
    videoscreenshotwidget.cpp \
    altitudeprofilewidget.cpp \
    videodetails.cpp \
    createnewcoursedialog.cpp \
    addsensorconfigurationdialog.cpp \
    videoqlistview.cpp

HEADERS  += mainwindow.h \
    videolistview.h \
    videolistmodel.h \
    videoitemdelegate.h \
    settingsdialog.h \
    videoscreenshotwidget.h \
    altitudeprofilewidget.h \
    videodetails.h \
    createnewcoursedialog.h \
    addsensorconfigurationdialog.h \
    videoqlistview.h

RESOURCES +=

FORMS += \
    settingsdialog.ui \
    videoscreenshotwidget.ui \
    altitudeprofilewidget.ui \
    videodetails.ui \
    createnewcoursedialog.ui \
    addsensorconfigurationdialog.ui

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

win32 {
    DEPLOY_COMMAND = windeployqt
    TARGET_CUSTOM_EXT = .exe
    CONFIG( debug, debug|release ) {
        # debug
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
    } else {
        # release
        DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
    }
    warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

    QMAKE_PRE_LINK = if not exist \"$${OUT_PWD}\\bin\" md \"$${OUT_PWD}\\bin\"
    QMAKE_POST_LINK = copy /y \"$${LIBAV_DLL_PATH}\\*.dll\" \"$${OUT_PWD}\\bin\\\" && \
                     copy /y \"$${LIBUSB_DLL}\" \"$${OUT_PWD}\\bin\\libusb0.dll\" && \
                   $${DEPLOY_COMMAND} $${DEPLOY_TARGET}

}
