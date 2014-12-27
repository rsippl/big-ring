include(../antlib/antlib.pri)

QT += core concurrent gui opengl widgets
CONFIG += link_pkgconfig

PKGCONFIG += glib-2.0 gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0

message("ant libs = $${ANTLIB_LIBRARY_DEPENDENCIES}")

message("compiling to architecture $${QMAKE_HOST.arch}")
contains(QMAKE_HOST.arch, x86_64):{
    MAINLIB_LIBRARY_DEPENCIES = $${ANTLIB_LIBRARY_DEPENDENCIES}
}
contains(QMAKE_HOST.arch, i386):{
    MAINLIB_LIBRARY_DEPENCIES = $${ANTLIB_LIBRARY_DEPENDENCIES}
}

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

