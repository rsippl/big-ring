include(../antlib/antlib.pri)

QMAKE_CXXFLAGS += -std=c++11
QT += core concurrent gui opengl widgets
CONFIG += link_pkgconfig

PKGCONFIG += glib-2.0 gstreamer-1.0
INCLUDEPATH += /home/ibooij/local/lib/qtgstreamer/include/Qt5GStreamer

message("ant libs = $${ANTLIB_LIBRARY_DEPENDENCIES}")

message("compiling to architecture $${QMAKE_HOST.arch}")
contains(QMAKE_HOST.arch, x86_64):{
    MAINLIB_LIBRARY_DEPENCIES = -L/home/ibooij/local/lib/qtgstreamer/lib/x86_64-linux-gnu -lQt5GLib-2.0 -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0 $${ANTLIB_LIBRARY_DEPENDENCIES}
}
contains(QMAKE_HOST.arch, i386):{
    MAINLIB_LIBRARY_DEPENCIES = -L/home/ibooij/local/lib/qtgstreamer/lib/i386-linux-gnu -lQt5GLib-2.0 -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0 $${ANTLIB_LIBRARY_DEPENDENCIES}
}

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

