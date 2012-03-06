project(RLVParserTest)
cmake_minimum_required(VERSION 2.8)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

include_directories(${QT_INCLUDE_DIR} ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_FLAGS "-W -Wall -Wextra -Werror")
set(SRCS main.cpp
        mainwindow.cpp
	reallivevideo.cpp
        reallivevideoimporter.cpp
        reallivevideowidget.cpp
        rlvlistwidget.cpp
        videowidget.cpp)

set(MOC_HEADERS
        mainwindow.h
        reallivevideoimporter.h
        reallivevideowidget.h
        rlvlistwidget.h
        videowidget.h)

set(UI_FILES
    rlvlistwidget.ui)

qt4_wrap_cpp(MOC_SRCS ${MOC_HEADERS})
qt4_wrap_ui(UI_SRCS ${UI_FILES})

add_executable(RLVParserTest ${SRCS} ${MOC_SRCS} ${UI_SRCS})
target_link_libraries(RLVParserTest ${QT_LIBRARIES} vlc)
