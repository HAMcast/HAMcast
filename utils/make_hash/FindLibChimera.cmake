# - Try to find libhamcast
# Once done this will define
#
#  CHIMERA_FOUND    - system has libchimera
#  CHIMERA_INCLUDE  - libchimera include dir
#  CHIMERA_LIBRARY  - link againgst libchimera
#

if (CHIMERA_LIBRARY AND CHIMERA_INCLUDE)
  set(CHIMERA_FOUND TRUE)
else (CHIMERA_LIBRARY AND CHIMERA_INCLUDE)

  find_path(CH_INCLUDE
    NAMES
      chimera/chimera.h
    PATHS
      ${CHIMERA_INCLUDE_PATH}
      ${CHIMERA_LIBRARY_PATH}
      ${CMAKE_INCLUDE_PATH}
      ${CMAKE_INSTALL_PREFIX}/include
      ../utils/chimera/include
      ../../utils/chimera/include
      ../../../utils/chimera/include
      ../../../../utils/chimera/include
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )
  
  if (CH_INCLUDE) 
    message (STATUS "Chimera header files found ...")
  else (CH_INCLUDE)
    message (SEND_ERROR "Chimera header files NOT found. Provide absolute path with -DCHIMERA_INCLUDE_PATH=<path-to-header>.")
  endif (CH_INCLUDE)

  find_library(CH_LIBRARY
    NAMES
      libchimera
      chimera
    PATHS
      ${CHIMERA_INCLUDE_PATH}
      ${CHIMERA_INCLUDE_PATH}/.libs
      ${CHIMERA_LIBRARY_PATH}
      ${CHIMERA_LIBRARY_PATH}/.libs
      ${CMAKE_LIBRARY_PATH}
      ${CMAKE_INSTALL_PREFIX}/lib
      ../utils/chimera/build
      ../../utils/chimera/build
      ../../../utils/chimera/build
      ../../../../utils/chimera/build
      ../lib
      ../../lib
      ../../../lib
      ../../../../lib
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  if (CH_LIBRARY) 
    message (STATUS "Chimera library found ...")
  else (CH_LIBRARY)
    message (SEND_ERROR "Chimera library NOT found. Provide absolute path with -DCHIMERA_LIBRARY_PATH=<path-to-library>.")
  endif (CH_LIBRARY)

  if (CH_INCLUDE AND CH_LIBRARY)
    set(CHIMERA_FOUND TRUE)
    set(CHIMERA_INCLUDE ${CH_INCLUDE})
    set(CHIMERA_LIBRARY ${CH_LIBRARY})
  else (CH_INCLUDE AND CH_LIBRARY)
    message (FATAL_ERROR "CHIMERA LIBRARY AND/OR HEADER NOT FOUND!")
  endif (CH_INCLUDE AND CH_LIBRARY)

endif (CHIMERA_LIBRARY AND CHIMERA_INCLUDE)
