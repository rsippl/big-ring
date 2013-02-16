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
find_package(LIBUSB-1 REQUIRED)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR} ${FFMPEG_INCLUDE_PATHS} ${GLEW_INCLUDE_PATH})
link_directories(${FFMPEG_LIBRARY_DIR})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror -D__STDC_CONSTANT_MACROS -DGLEW_STATIC")
if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
endif()

set(ANT_SRCS
	antdevice.cpp
	antdevicefinder.cpp
	ANT.cpp
	ANTChannel.cpp
	antcontroller.cpp
	ANTMessage.cpp
	usb2antdevice.cpp)

if (WIN32)
	set(USBXPRESS_INCLUDE_PATH "D:/SiLabs/MCU/USBXpress/USBXpress_API/Host")
	set(USBXPRESS_LIB_DIR "D:/SiLabs/MCU/USBXpress/USBXpress_API/Host/x86")
	set(ANT_SRCS ${ANT_SRCS} usbexpressantdevice.cpp)
	set(EXTRA_LIBRARIES opengl32 SiUSBXp)
else()
	set(ANT_SRCS ${ANT_SRCS} unixserialusbant.cpp)
endif()

set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${LIBUSB_1_LIBRARIES})
include_directories(${QT_INCLUDE_DIR} ${FFMPEG_INCLUDE_PATHS} ${USBXPRESS_INCLUDE_PATH})
link_directories(${FFMPEG_LIBRARY_DIR} ${USBXPRESS_LIB_DIR})

set(SRCS
	 cyclist.cpp
	 mainwindow.cpp
	 reallivevideo.cpp
	 reallivevideoimporter.cpp
	 rlvfileparser.cpp
	 rlvlistwidget.cpp
	 simulation.cpp
	 videocontroller.cpp
	 videodecoder.cpp
	 videowidget.cpp

	 thirdparty/src/glew.c)

set(UI_FILES
	rlvlistwidget.ui)

add_definitions(-DGLEW_STATIC)
qt4_wrap_ui(UI_SRCS ${UI_FILES})

add_library(mainlib ${ANT_SRCS} ${SRCS} ${UI_SRCS})

add_executable(indoorcycling main.cpp)
target_link_libraries(indoorcycling mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES} ${EXTRA_LIBRARIES})

# test executables
add_executable(test_modelclasses reallivevideotest.cpp)
target_link_libraries(test_modelclasses mainlib ${QT_LIBRARIES} ${EXTRA_LIBRARIES})

add_executable(rlvtest rlvtestmain.cpp rlvtester.cpp)
target_link_libraries(rlvtest mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES} ${EXTRA_LIBRARIES})

