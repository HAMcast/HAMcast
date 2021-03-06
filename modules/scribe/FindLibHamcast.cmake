# - Try to find libhamcast
# Once done this will define
#
#  HAMCAST_FOUND    - system has libhamcast
#  HAMCAST_INCLUDE  - libhamcast include dir
#  HAMCAST_LIBRARY  - link againgst libhamcast
#

if (HAMCAST_LIBRARY AND HAMCAST_INCLUDE)
  set(HAMCAST_FOUND TRUE)
else (HAMCAST_LIBRARY AND HAMCAST_INCLUDE)

  find_path(HC_INCLUDE
    NAMES
      hamcast/hamcast.hpp
    PATHS
      ${HAMCAST_INCLUDE_PATH}
      ${HAMCAST_LIBRARY_PATH}
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
      ../libhamcast
      ../../libhamcast
      ../../../libhamcast
      ../../../../libhamcast
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )
  
  if (HC_INCLUDE) 
    message (STATUS "HAMcast header files found ...")
  else (HC_INCLUDE)
    message (SEND_ERROR "HAMcast header files NOT found. Provide absolute path with -DHAMCAST_INCLUDE_PATH=<path-to-header>.")
  endif (HC_INCLUDE)

  find_library(HC_LIBRARY
    NAMES
      libhamcast
      hamcast
    PATHS
      ${HAMCAST_INCLUDE_PATH}
      ${HAMCAST_INCLUDE_PATH}/.libs
      ${HAMCAST_LIBRARY_PATH}
      ${HAMCAST_LIBRARY_PATH}/.libs
      ${CMAKE_LIBRARY_PATH}
      ${CMAKE_INSTALL_PREFIX}/lib
      ../libhamcast/build
      ../../libhamcast/build
      ../../../libhamcast/build
      ../../../../libhamcast/build
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ../lib
      ../../lib
      ../../../lib
      ../../../../lib
  )

  if (HC_LIBRARY) 
    message (STATUS "HAMcast library found ...")
  else (HC_LIBRARY)
    message (SEND_ERROR "HAMcast library NOT found. Provide absolute path with -DHAMCAST_LIBRARY_PATH=<path-to-library>.")
  endif (HC_LIBRARY)

  if (HC_INCLUDE AND HC_LIBRARY)
    set(HAMCAST_FOUND TRUE)
    set(HAMCAST_INCLUDE ${HC_INCLUDE})
    set(HAMCAST_LIBRARY ${HC_LIBRARY})
  else (HC_INCLUDE AND HC_LIBRARY)
    message (FATAL_ERROR "HAMCAST LIBRARY AND/OR HEADER NOT FOUND!")
  endif (HC_INCLUDE AND HC_LIBRARY)

endif (HAMCAST_LIBRARY AND HAMCAST_INCLUDE)
