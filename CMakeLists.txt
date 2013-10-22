project(IndoorCycling)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
SET(QT_USE_QTOPENGL TRUE)
SET(QT_USE_QTTEST TRUE)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(OpenGL)
find_package(Threads)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(FFMPEG REQUIRED)
find_package(LIBUSB-1 REQUIRED)
include_directories(${Qt5Concurrent_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Quick_INCLUDE_DIRS} ${FFMPEG_INCLUDE_PATHS} ${CMAKE_SOURCE_DIR}/thirdparty/include ${LIBUSB_1_INCLUDE_DIRS})
link_directories(${FFMPEG_LIBRARY_DIR})
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5OpenGL_DEFINITIONS})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror -std=c++11 ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} -D__STDC_CONSTANT_MACROS -DGLEW_STATIC")
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
	antmessagegatherer.cpp
	usb2antdevice.cpp)

if (WIN32)
	set(USBXPRESS_INCLUDE_PATH "D:/SiLabs/MCU/USBXpress/USBXpress_API/Host")
	set(USBXPRESS_LIB_DIR "D:/SiLabs/MCU/USBXpress/USBXpress_API/Host/x86")
	set(ANT_SRCS ${ANT_SRCS} usbexpressantdevice.cpp)
	set(EXTRA_LIBRARIES opengl32 SiUSBXp)
else()
	set(ANT_SRCS ${ANT_SRCS} unixserialusbant.cpp)
endif()

set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} ${LIBUSB_1_LIBRARIES} ${OPENGL_LIBRARY})
include_directories(${QT_INCLUDE_DIR} ${FFMPEG_INCLUDE_PATHS} ${USBXPRESS_INCLUDE_PATH})
link_directories(${FFMPEG_LIBRARY_DIR} ${USBXPRESS_LIB_DIR})

set(SRCS
	 cyclist.cpp
	 mainwindow.cpp
	 profile.cpp
	 reallifevideo.cpp
	 reallifevideoimporter.cpp
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
qt5_wrap_ui(UI_SRCS ${UI_FILES})

set(QT_LIBRARIES
	 ${Qt5OpenGL_LIBRARIES}
	 ${Qt5Concurrent_LIBRARIES}
	 ${Qt5Widgets_LIBRARIES}
	 ${Qt5Qml_LIBRARIES}
	 ${Qt5Quick_LIBRARIES})

add_library(antlib ${ANT_SRCS})
add_library(mainlib ${SRCS} ${UI_SRCS})

qt5_add_resources(file_resources resources.qrc)

add_executable(indoorcycling main.cpp ${file_resources})
#qt5_use_modules(indoorcycling Widgets)
target_link_libraries(indoorcycling antlib mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES} ${EXTRA_LIBRARIES})

add_executable(rlvtest rlvtestmain.cpp rlvtester.cpp)
target_link_libraries(rlvtest mainlib ${QT_LIBRARIES} ${FFMPEG_LIBRARIES} ${EXTRA_LIBRARIES})

# for Sufferfest Player
add_executable(sufferfestplayer sufferfestplayer.cpp game.cpp moviefiledroparea.cpp ${file_resources})
target_link_libraries(sufferfestplayer antlib ${EXTRA_LIBRARIES} ${QT_LIBRARIES})

# Prepare for testing.
#add_subdirectory(ext/gtest)
#enable_testing()
#include_directories(${GTEST_INCLUDE_DIRS})

#file(GLOB TEST_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*test.cpp)
#add_executable(indoorcycling_tests ${TEST_SRC_FILES})
#add_dependencies(indoorcycling_tests googletest)
#target_link_libraries(indoorcycling_tests antlib mainlib
#	${GTEST_LIBS_DIR}/libgtest.a
#	${GTEST_LIBS_DIR}/libgtest_main.a
#	${QT_LIBRARIES} ${Qt5Test_LIBRARIES} ${FFMPEG_LIBRARIES} ${EXTRA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
#add_test(indoorcycling_tests indoorcycling_tests)



