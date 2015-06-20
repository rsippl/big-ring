# if installed, point to the source directory of google test here.
GOOGLE_TEST_SOURCE_DIR=/usr/src/gtest

QMAKE_CXXFLAGS += -std=c++11 -W -Wall -Wextra -Werror

linux {
    PKGCONFIG += libavcodec libavformat libavutil libswscale
}
win32 {
    INCLUDEPATH += C:\development\libav-i686-w64-mingw32-11.2\usr\include
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


