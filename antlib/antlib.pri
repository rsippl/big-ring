include(../config.pri)

message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

ANTLIB_LIBRARY_DEPENDENCIES = -lusb-1.0

