TEMPLATE = app

include(../mainlib/Defines.pri)

TARGET = indoorcycling
SOURCES += main.cpp

LIBS += $$IN_PWD/../mainlib/libmainlib.a -lusb-1.0 -lavcodec -lavformat -lavutil
