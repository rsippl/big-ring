# if installed, point to the source directory of google test here.

QMAKE_CXXFLAGS += -std=c++11 -W -Wall -Wextra -Werror

linux {
    PKGCONFIG += libavcodec libavformat libavutil libswscale
}
win32 {
    LIBAV_PATH=C:/development/libav-i686-w64-mingw32-11.2
    LIBAV_DLL_PATH = $$LIBAV_PATH/usr/bin
    INCLUDEPATH += $$LIBAV_PATH\usr\include
    LIBS += $$LIBAV_PATH\usr\bin\avcodec-56.dll
    LIBS += $$LIBAV_PATH\usr\bin\avformat-56.dll
    LIBS += $$LIBAV_PATH\usr\bin\avutil-54.dll
    LIBS += $$LIBAV_PATH\usr\bin\swscale-3.dll
}
# address sanitizer configuration. Uncomment this to build
# with address sanitizer.
#QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_CFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS+= -fsanitize=address
#LIBS += -Wl,--no-as-needed -lasan -Wl,--as-needed

# profiler configuration. Uncomment this to use google profiler.
#LIBS += -Wl,--no-as-needed -lprofiler -Wl,--as-needed
#LIBS += -Wl,--no-as-needed -ltcmalloc -Wl,--as-needed


