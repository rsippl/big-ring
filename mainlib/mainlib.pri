include(../antlib/antlib.pri)

QT += core concurrent gui opengl widgets svg
CONFIG += link_pkgconfig

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

RESOURCES += \
    $$IN_PWD/shaders.qrc \
    $$IN_PWD/flags.qrc \



