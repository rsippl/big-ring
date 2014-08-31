QMAKE_CXXFLAGS += -std=c++11
QT += core concurrent gui opengl widgets

LIBS += -lusb-1.0 -lavcodec -lavformat -lavutil

RESOURCES += \
    $$IN_PWD/shaders.qrc

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

