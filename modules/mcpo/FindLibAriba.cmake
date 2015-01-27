# - Try to find libhamcast
# Once done this will define
#
#  ARIBA_FOUND    - system has libhamcast
#  ARIBA_INCLUDE  - libhamcast include dir
#  ARIBA_LIBRARY  - link againgst libhamcast
#

if (ARIBA_LIBRARY AND ARIBA_INCLUDE)
  set(ARIBA_FOUND TRUE)
else (ARIBA_LIBRARY AND ARIBA_INCLUDE)

  find_path(A_INCLUDE
    NAMES
      ariba/ariba.h
    PATHS
    ${ARIBA_INCLUDE_PATH}
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ./ariba/source
      ../ariba/source
      ${ARIBA_LIBRARY_PATH}
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
  )
  
  if (A_INCLUDE) 
    message (STATUS "Ariba header files found at..."${A_INCLUDE})
  else (A_INCLUDE)
    message (STATUS "Header files NOT found. Provide path with -DARIBA_INCLUDE_PATH=<path-to-header>.")
  endif (A_INCLUDE)

  find_library(A_LIBRARY
    NAMES
      libariba
      ariba
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ./ariba/source/ariba/.libs
      ../ariba/source/ariba/.libs
      ${ARIBA_INCLUDE_PATH}
      ${ARIBA_INCLUDE_PATH}/ariba/.libs
      ${ARIBA_LIBRARY_PATH}
      ${ARIBA_LIBRARY_PATH}/.libs
      ${CMAKE_LIBRARY_PATH}
      ${CMAKE_INSTALL_PREFIX}/lib
  )

  if (A_LIBRARY) 
    message (STATUS "Ariba Library found ...")
  else (A_LIBRARY)
    message (STATUS "Library NOT found. Provide path with -DARIBA_LIBRARY_PATH=<path-to-library>.")
  endif (A_LIBRARY)

  if (A_INCLUDE AND A_LIBRARY)
    set(ARIBA_FOUND TRUE)
    set(ARIBA_INCLUDE ${A_INCLUDE})
    set(ARIBA_LIBRARY ${A_LIBRARY})
  else (A_INCLUDE AND A_LIBRARY)
    message (FATAL_ERROR "ARIBA LIBRARY AND/OR HEADER NOT FOUND!")
  endif (A_INCLUDE AND A_LIBRARY)

endif (ARIBA_LIBRARY AND ARIBA_INCLUDE)
