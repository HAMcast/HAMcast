# - Try to find libhamcast
# Once done this will define
#
#  MCPO_FOUND    - system has libhamcast
#  MCPO_INCLUDE  - libhamcast include dir
#  MCPO_LIBRARY  - link againgst libhamcast
#

if (MCPO_LIBRARY AND MCPO_INCLUDE)
  set(MCPO_FOUND TRUE)
else (MCPO_LIBRARY AND MCPO_INCLUDE)

  find_path(M_INCLUDE
    NAMES
      mcpo/MCPO.h
    PATHS

      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
      ./mcpo/source
      ../mcpo/source
      ${MCPO_INCLUDE_PATH}
      ${MCPO_LIBRARY_PATH}
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
  )
  
  if (M_INCLUDE) 
    message (STATUS " Mcpo Header files found ...")
  else (M_INCLUDE)
    message (STATUS "Header files NOT found. Provide path with -DMCPO_INCLUDE_PATH=<path-to-header>.")
  endif (M_INCLUDE)

  find_library(M_LIBRARY
    NAMES
      libmcpo
      mcpo
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
      ./mcpo/source/mcpo/.libs
      ../mcpo/source/mcpo/.libs
      ${MCPO_INCLUDE_PATH}
      ${MCPO_INCLUDE_PATH}/mcpo/.libs
      ${MCPO_LIBRARY_PATH}
      ${MCPO_LIBRARY_PATH}/.libs
      ${CMAKE_LIBRARY_PATH}
      ${CMAKE_INSTALL_PREFIX}/lib
  )

  if (M_LIBRARY) 
    message (STATUS "MCPO Library found ...")
  else (M_LIBRARY)
    message (STATUS "Library NOT found. Provide path with -DMCPO_LIBRARY_PATH=<path-to-library>.")
  endif (M_LIBRARY)

  if (M_INCLUDE AND M_LIBRARY)
    set(MCPO_FOUND TRUE)
    set(MCPO_INCLUDE ${M_INCLUDE})
    set(MCPO_LIBRARY ${M_LIBRARY})
  else (M_INCLUDE AND M_LIBRARY)
    message (FATAL_ERROR "MCPO LIBRARY AND/OR HEADER NOT FOUND!")
  endif (M_INCLUDE AND M_LIBRARY)

endif (MCPO_LIBRARY AND MCPO_INCLUDE)
