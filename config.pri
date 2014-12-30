# if installed, point to the source directory of google test here.
GOOGLE_TEST_SOURCE_DIR=/usr/src/gtest

QMAKE_CXXFLAGS += -std=c++11

# address sanitizer configuration
#QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_CFLAGS+= -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS+= -fsanitize=address
#LIBS += -Wl,--no-as-needed -lasan -Wl,--as-needed

# profiler configuration
#LIBS += -Wl,--no-as-needed -lprofiler -Wl,--as-needed

