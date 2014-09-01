QMAKE_CXXFLAGS += -std=c++11

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

ANTLIB_LIBRARY_DEPENDENCIES = -lusb-1.0

