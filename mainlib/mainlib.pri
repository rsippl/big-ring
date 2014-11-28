include(../antlib/antlib.pri)

QMAKE_CXXFLAGS += -std=c++11
QT += core concurrent gui opengl widgets

INCLUDEPATH += /home/ibooij/local/lib/qtgstreamer/include/Qt5GStreamer

message("ant libs = $${ANTLIB_LIBRARY_DEPENDENCIES}")

MAINLIB_LIBRARY_DEPENCIES = -L/home/ibooij/local/lib/qtgstreamer/lib/i386-linux-gnu -lQt5GLib-2.0 -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0 $${ANTLIB_LIBRARY_DEPENDENCIES}

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

