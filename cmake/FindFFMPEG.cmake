# - Try to find FFMPEG
# Once done this will define
#
#  FFMPEG_FOUND		 - system has FFMPEG
#  FFMPEG_INCLUDE_DIR	 - the include directories
#  FFMPEG_LIBRARY_DIR	 - the directory containing the libraries
#  FFMPEG_LIBRARIES	 - link these to use FFMPEG
#

SET( FFMPEG_HEADERS avformat.h avcodec.h avutil.h avdevice.h)
SET( FFMPEG_PATH_SUFFIXES libavformat libavcodec libavutil libavdevice libffmpeg )
MESSAGE("dir set to $ENV{FFMPEGDIR}")
MESSAGE("dir set to ${FFMPEGDIR}")
if( WIN32 )
   SET( FFMPEG_LIBRARIES avformat.lib avcodec.lib avutil.lib avdevice.lib swscale.lib)
   SET( FFMPEG_LIBRARY_DIR ${FFMPEGDIR}\\lib )
   SET( FFMPEG_INCLUDE_PATHS ${FFMPEGDIR}\\include )

   # check to see if we can find swscale
   SET( TMP_ TMP-NOTFOUND )
	FIND_PATH(
		  PATHS ${FFMPEG_LIBRARY_DIR} )
else( WIN32 )
   SET( FFMPEG_LIBRARIES avformat avcodec avutil avdevice )
   INCLUDE(FindPkgConfig)
   if ( PKG_CONFIG_FOUND )
	  pkg_check_modules( AVFORMAT libavformat )
	  pkg_check_modules( AVCODEC libavcodec )
	  pkg_check_modules( AVUTIL libavutil )
	  pkg_check_modules( AVDEVICE libavdevice )
   endif ( PKG_CONFIG_FOUND )

   SET( FFMPEG_LIBRARY_DIR   ${AVFORMAT_LIBRARY_DIRS}
				 ${AVCODEC_LIBRARY_DIRS}
				 ${AVUTIL_LIBRARY_DIRS}
				 ${AVDEVICE_LIBRARY_DIRS} )
   SET( FFMPEG_INCLUDE_PATHS ${AVFORMAT_INCLUDE_DIRS}
				 ${AVCODEC_INCLUDE_DIRS}
				 ${AVUTIL_INCLUDE_DIRS}
				 ${AVDEVICE_INCLUDE_DIRS} )
endif( WIN32 )

# find includes
SET( INC_SUCCESS 0 )
SET( TMP_ TMP-NOTFOUND )
SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_PATHS} )
FOREACH( INC_ ${FFMPEG_HEADERS} )
   message( "checking: " ${INC_} )

   FIND_PATH( TMP_ ${INC_}
		  PATHS ${FFMPEG_INCLUDE_PATHS}
		  PATH_SUFFIXES ${FFMPEG_PATH_SUFFIXES} )
   IF ( TMP_ )
	  message( "found: " ${TMP_} )
	  MATH( EXPR INC_SUCCESS ${INC_SUCCESS}+1 )
	  SET( FFMPEG_INCLUDE_DIR ${FFMPEG_INCLUDE_DIR} ${TMP_} )
   ENDIF ( TMP_ )
   SET( TMP_ TMP-NOTFOUND )
ENDFOREACH( INC_ )

# clear out duplicates
if(NOT "${FFMPEG_INCLUDE_DIR}" MATCHES "")
	LIST( REMOVE_DUPLICATES FFMPEG_INCLUDE_DIR )
endif(NOT "${FFMPEG_INCLUDE_DIR}" MATCHES "")
if(NOT "${FFMPEG_LIBRARY_DIR}" MATCHES "")
	LIST( REMOVE_DUPLICATES FFMPEG_LIBRARY_DIR )
endif(NOT "${FFMPEG_LIBRARY_DIR}" MATCHES "")

# find the full paths of the libraries
SET( TMP_ TMP-NOTFOUND )
IF ( NOT WIN32 )
   FOREACH( LIB_ ${FFMPEG_LIBRARIES} )
	  FIND_LIBRARY( TMP_ NAMES ${LIB_} PATHS ${FFMPEG_LIBRARY_DIR} )
	  IF ( TMP_ )
	 SET( FFMPEG_LIBRARIES_FULL ${FFMPEG_LIBRARIES_FULL} ${TMP_} )
	  ENDIF ( TMP_ )
	  SET( TMP_ TMP-NOTFOUND )
   ENDFOREACH( LIB_ )
   SET ( FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES_FULL} )
ENDIF( NOT WIN32 )

LIST( LENGTH FFMPEG_HEADERS LIST_SIZE_ )

SET( FFMPEG_FOUND FALSE )
IF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )
   SET( FFMPEG_FOUND TRUE )
ENDIF ( ${INC_SUCCESS} EQUAL ${LIST_SIZE_} )
MESSAGE("HEADERS IN ${FFMPEG_INCLUDE_PATHS}")
MESSAGE("LIBS IN ${FFMPEG_LIBRARY_DIR}")
