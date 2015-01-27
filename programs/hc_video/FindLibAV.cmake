# - Try to find libav*
# Once done this will define
#
#  LIBAV_FOUND          - system has libav*
#  LIBAV_INCLUDE_DIRS   - the libav* include directories
#  LIBAV_LIBRARIES      - Link these to use libav*
#  LIBAV_DEFINITIONS    - Compiler switches required for using libav*
#
#

if (LIBAV_LIBRARIES AND LIBAV_INCLUDE_DIRS)
  # in cache already
  set(LIBAV_FOUND TRUE)
else (LIBAV_LIBRARIES AND LIBAV_INCLUDE_DIRS)
  # use pkg-config to get the directories and then use these values
  # in the FIND_PATH() and FIND_LIBRARY() calls
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_LIBAV_CODEC libavcodec)
    pkg_check_modules(_LIBAV_DEVICE libavdevice)
    pkg_check_modules(_LIBAV_FILTER libavfilter)
    pkg_check_modules(_LIBAV_FORMAT libavformat)
    pkg_check_modules(_LIBAV_UTIL libavutil)
    pkg_check_modules(_LIBAV_SWSCALE libswscale)
  endif (PKG_CONFIG_FOUND)
  
  message(STATUS "libav* pkg-config paths:")
  message(STATUS "   libavcodec:   ${_LIBAV_CODEC_PREFIX}")
  message(STATUS "   libavdevice:  ${_LIBAV_DEVICE_PREFIX}")
  message(STATUS "   libavfilter:  ${_LIBAV_FILTER_PREFIX}")
  message(STATUS "   libavformat:  ${_LIBAV_FORMAT_PREFIX}")
  message(STATUS "   libavutil:    ${_LIBAV_UTIL_PREFIX}")
  message(STATUS "   libavswscale: ${_LIBAV_SWSCALE_PREFIX}")
  
  find_path(LIBAV_CODEC_INCLUDE_DIR
    NAMES libavcodec/avcodec.h
    HINTS ${_LIBAV_CODEC_INCLUDE_DIRS} ${_LIBAV_CODEC_PREFIX}/include
  )
  
  find_path(LIBAV_DEVICE_INCLUDE_DIR
    NAMES libavdevice/avdevice.h
    HINTS ${_LIBAV_DEVICE_INCLUDE_DIRS} ${_LIBAV_DEVICE_PREFIX}/include 
  )
  
  find_path(LIBAV_FILTER_INCLUDE_DIR
    NAMES libavfilter/avfilter.h
    HINTS ${_LIBAV_FILTER_INCLUDE_DIRS} ${_LIBAV_FILTER_PREFIX}/include 
  )

  find_path(LIBAV_FORMAT_INCLUDE_DIR
    NAMES libavformat/avformat.h
    HINTS ${_LIBAV_FORMAT_INCLUDE_DIRS} ${_LIBAV_FORMAT_PREFIX}/include 
  )
  
  find_path(LIBAV_UTIL_INCLUDE_DIR
    NAMES libavutil/avutil.h
    HINTS ${_LIBAV_UTIL_INCLUDE_DIRS} ${_LIBAV_UTIL_PREFIX}/include 
  )

  find_path(LIBAV_SWSCALE_INCLUDE_DIR
    NAMES libswscale/swscale.h
    HINTS ${_LIBAV_SWSCALE_INCLUDE_DIRS} ${_LIBAV_SWSCALE_PREFIX}/include 
  )
  
  find_library(LIBAV_UTIL_LIBRARY
    NAMES avutil
    HINTS ${_LIBAV_UTIL_LIBRARY_DIRS} ${_LIBAV_UTIL_PREFIX}/lib 
  )
  
  find_library(LIBAV_CODEC_LIBRARY
    NAMES avcodec
    HINTS ${_LIBAV_CODEC_LIBRARY_DIRS} ${_LIBAV_CODEC_PREFIX}/lib 
  )
  
  find_library(LIBAV_DEVICE_LIBRARY
    NAMES avdevice
    HINTS ${_LIBAV_DEVICE_LIBRARY_DIRS} ${_LIBAV_DEVICE_PREFIX}/lib 
  )
  
  find_library(LIBAV_FILTER_LIBRARY
    NAMES avfilter
    HINTS ${_LIBAV_FILTER_LIBRARY_DIRS} ${_LIBAV_FILTER_PREFIX}/lib 
  )
  
  find_library(LIBAV_FORMAT_LIBRARY
    NAMES avformat
    HINTS ${_LIBAV_FORMAT_LIBRARY_DIRS} ${_LIBAV_FORMAT_PREFIX}/lib  
  )
  
  find_library(LIBAV_SWSCALE_LIBRARY
    NAMES swscale
    HINTS ${_LIBAV_SWSCALE_LIBRARY_DIRS} ${_LIBAV_SWSCALE_PREFIX}/lib 
  )
  
  
  if (LIBAV_CODEC_INCLUDE_DIR AND 
        LIBAV_DEVICE_INCLUDE_DIR AND 
        LIBAV_FILTER_INCLUDE_DIR AND 
        LIBAV_FORMAT_INCLUDE_DIR AND 
        LIBAV_UTIL_INCLUDE_DIR AND 
        LIBAV_SWSCALE_INCLUDE_DIR AND
        LIBAV_CODEC_LIBRARY AND 
        LIBAV_DEVICE_LIBRARY AND 
        LIBAV_FILTER_LIBRARY AND 
        LIBAV_FORMAT_LIBRARY AND 
        LIBAV_UTIL_LIBRARY AND 
        LIBAV_SWSCALE_LIBRARY )
    set(LIBAV_FOUND TRUE)
  endif(LIBAV_CODEC_INCLUDE_DIR AND 
        LIBAV_DEVICE_INCLUDE_DIR AND 
        LIBAV_FILTER_INCLUDE_DIR AND 
        LIBAV_FORMAT_INCLUDE_DIR AND 
        LIBAV_UTIL_INCLUDE_DIR AND 
        LIBAV_SWSCALE_INCLUDE_DIR AND
        LIBAV_CODEC_LIBRARY AND 
        LIBAV_DEVICE_LIBRARY AND 
        LIBAV_FILTER_LIBRARY AND 
        LIBAV_FORMAT_LIBRARY AND 
        LIBAV_UTIL_LIBRARY AND 
        LIBAV_SWSCALE_LIBRARY)

  if (LIBAV_FOUND)
    set(LIBAV_INCLUDE_DIRS
      ${LIBAV_CODEC_INCLUDE_DIR}
      ${LIBAV_DEVICE_INCLUDE_DIR}
      ${LIBAV_FILTER_INCLUDE_DIR}
      ${LIBAV_FORMAT_INCLUDE_DIR}
      ${LIBAV_UTIL_INCLUDE_DIR}
      ${LIBAV_SWSCALE_INCLUDE_DIR}
    )

    set(LIBAV_LIBRARIES
      ${LIBAV_UTIL_LIBRARY}      
      ${LIBAV_CODEC_LIBRARY}
      ${LIBAV_FORMAT_LIBRARY}
      ${LIBAV_SWSCALE_LIBRARY}
      ${LIBAV_FILTER_LIBRARY}
      ${LIBAV_DEVICE_LIBRARY}
    )
  endif (LIBAV_FOUND)

  if (LIBAV_FOUND)
    if (NOT LIBAV_FIND_QUIETLY)
      message(STATUS "Found LIBAV:")
      message(STATUS "   ${LIBAV_CODEC_INCLUDE_DIR} ")
      message(STATUS "   ${LIBAV_DEVICE_INCLUDE_DIR} ")
      message(STATUS "   ${LIBAV_FILTER_INCLUDE_DIR} ")
      message(STATUS "   ${LIBAV_FORMAT_INCLUDE_DIR} ")
      message(STATUS "   ${LIBAV_UTIL_INCLUDE_DIR} ")
      message(STATUS "   ${LIBAV_SWSCALE_INCLUDE_DIR} ")
      
      message(STATUS "   ${LIBAV_SWSCALE_LIBRARY} ")
      message(STATUS "   ${LIBAV_FILTER_LIBRARY} ")
      message(STATUS "   ${LIBAV_DEVICE_LIBRARY} ")
      message(STATUS "   ${LIBAV_FORMAT_LIBRARY} ")
      message(STATUS "   ${LIBAV_CODEC_LIBRARY} ")
      message(STATUS "   ${LIBAV_UTIL_LIBRARY}   ")
    endif (NOT LIBAV_FIND_QUIETLY)
  else (LIBAV_FOUND)
    if (LIBAV_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find libav*")
    endif (LIBAV_FIND_REQUIRED)
  endif (LIBAV_FOUND)

  mark_as_advanced(LIBAV_INCLUDE_DIRS LIBAV_LIBRARIES)

endif (LIBAV_LIBRARIES AND LIBAV_INCLUDE_DIRS)

