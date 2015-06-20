include(../config.pri)

linux {
    PKGCONFIG += libusb-1.0
}
win32 {
    INCLUDEPATH += C:\development\libusb-win32-bin-1.2.6.0\include
    LIBS += C:\development\libusb-win32-bin-1.2.6.0\bin\x86\libusb0_x86.dll
}
message(Including $$_FILE_ from $$IN_PWD)
INCLUDEPATH += $$IN_PWD/

