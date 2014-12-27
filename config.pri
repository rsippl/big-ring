# if installed, point to the source directory of google test here.
GOOGLE_TEST_SOURCE_DIR=/usr/src/gtest

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS+= -fsanitize=address -fno-omit-frame-pointer
QMAKE_CFLAGS+= -fsanitize=address -fno-omit-frame-pointer
QMAKE_LFLAGS+= -fsanitize=address

