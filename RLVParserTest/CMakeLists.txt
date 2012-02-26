project(RLVParserTest)
cmake_minimum_required(VERSION 2.8)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

include_directories(${QT_INCLUDE_DIR})

set(SRCS main.cpp
	reallivevideo.cpp
	reallivevideoparser.cpp
	reallivevideowidget.cpp)

set(MOC_HEADERS
	reallivevideowidget.h)

qt4_wrap_cpp(MOC_SRCS ${MOC_HEADERS})

add_executable(RLVParserTest ${SRCS} ${MOC_SRCS})
target_link_libraries(RLVParserTest ${QT_LIBRARIES})
