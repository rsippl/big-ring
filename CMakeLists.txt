project(IndoorCycling)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTTEST TRUE)
find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui REQUIRED)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(FFMPEG REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror -D__STDC_CONSTANT_MACROS -pg")
set(ANT_SRCS
	ANT.cpp
	ANTChannel.cpp
	antcontroller.cpp
	ANTMessage.cpp
	unixserialusbant.cpp)

set(SRCS
	 cyclist.cpp
	 mainwindow.cpp
	 reallivevideo.cpp
	 reallivevideoimporter.cpp
	 reallivevideowidget.cpp
	 rlvfileparser.cpp
	 rlvlistwidget.cpp
	 simulation.cpp
	 videocontroller.cpp
	 videodecoder.cpp
	 videowidget.cpp)

set(UI_FILES
    rlvlistwidget.ui)

qt4_wrap_ui(UI_SRCS ${UI_FILES})

add_library(mainlib ${ANT_SRCS} ${SRCS} ${UI_SRCS})

add_executable(indoorcycling main.cpp)
target_link_libraries(indoorcycling mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES})

# test executables
add_executable(test_modelclasses reallivevideotest.cpp)
target_link_libraries(test_modelclasses mainlib ${QT_LIBRARIES})

add_executable(rlvtest rlvtestmain.cpp rlvtester.cpp)
target_link_libraries(rlvtest mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES})

