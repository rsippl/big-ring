include(../antlib/antlib.pri)

QMAKE_CXXFLAGS += -std=c++11
QT += core concurrent gui opengl widgets

message("ant libs = $${ANTLIB_LIBRARY_DEPENDENCIES}")
MAINLIB_LIBRARY_DEPENCIES = -lavcodec -lavformat -lavutil $${ANTLIB_LIBRARY_DEPENDENCIES}

RESOURCES += \
    $$IN_PWD/shaders.qrc

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

