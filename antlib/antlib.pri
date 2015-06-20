include(../config.pri)

linux {
    PKGCONFIG += libusb-1.0
}
win32 {
    INCLUDEPATH += C:\development\libusb-win32-bin-1.2.6.0\include
}
message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

