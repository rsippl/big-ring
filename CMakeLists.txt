project(IndoorCycling)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui REQUIRED)


find_package(FFMPEG REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDE_DIR} /usr/include/phonon ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror -D__STDC_CONSTANT_MACROS")
set(SRCS main.cpp
        mainwindow.cpp
		reallivevideo.cpp
        reallivevideoimporter.cpp
        reallivevideowidget.cpp
        rlvfileparser.cpp
        rlvlistwidget.cpp
        videodecoder.cpp
		videowidget.cpp)

set(MOC_HEADERS
        mainwindow.h
        reallivevideoimporter.h
        reallivevideowidget.h
        rlvlistwidget.h
        videodecoder.h
		videowidget.h)

set(UI_FILES
    rlvlistwidget.ui)

qt4_wrap_cpp(MOC_SRCS ${MOC_HEADERS})
qt4_wrap_ui(UI_SRCS ${UI_FILES})

add_executable(indoorcycling ${SRCS} ${MOC_SRCS} ${UI_SRCS})
target_link_libraries(indoorcycling ${QT_LIBRARIES} phonon ${FFMPEG_LIBRARIES})
