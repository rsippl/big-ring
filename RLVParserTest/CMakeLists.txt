project(RLVParserTest)
cmake_minimum_required(VERSION 2.8)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

include_directories(${QT_INCLUDE_DIR})

set(SRCS main.cpp
	reallivevideo.cpp
	reallivevideoparser.cpp)


add_executable(RLVParserTest ${SRCS})
target_link_libraries(RLVParserTest ${QT_LIBRARIES})
