project(IndoorCycling)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(QT_USE_QTOPENGL TRUE)
find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(FFMPEG REQUIRED)
include(${QT_USE_FILE})

include_directories(${QT_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror -D__STDC_CONSTANT_MACROS")
set(ANT_SRCS
	ANT.cpp
	ANTChannel.cpp
	antcontroller.cpp
	ANTMessage.cpp
	CommPort.cpp
	Serial.cpp)

set(SRCS main.cpp
        mainwindow.cpp
		reallivevideo.cpp
        reallivevideoimporter.cpp
        reallivevideowidget.cpp
        rlvfileparser.cpp
        rlvlistwidget.cpp
		videocontroller.cpp
		videodecoder.cpp
		videowidget.cpp)

set(MOC_HEADERS
        mainwindow.h
        reallivevideoimporter.h
        reallivevideowidget.h
        rlvlistwidget.h
		videocontroller.h
		videodecoder.h
		videowidget.h)

set(UI_FILES
    rlvlistwidget.ui)

qt4_wrap_ui(UI_SRCS ${UI_FILES})

add_executable(indoorcycling ${ANT_SRCS} ${SRCS} ${UI_SRCS})
target_link_libraries(indoorcycling ${QT_LIBRARIES} QtMultimediaKit ${FFMPEG_LIBRARIES})
