cmake_minimum_required(VERSION 2.6)
project (hamcast_libchimera)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (LIBCHIMERA_INCLUDE include/chimera)
set (LIBCHIMERA_SRC src/chimera.c src/network.c src/message.c src/log.c
                    src/jrb.c src/jval.c src/key.c src/route.c src/host.c
                    src/dllist.c src/job_queue.c src/dtime.c src/semaphore.c)

# search for libs
find_package (Threads REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories (${LIBCHIMERA_INCLUDE} ${OPENSSL_INCLUDE_DIR})

add_library(libchimera SHARED ${LIBCHIMERA_SRC})

target_link_libraries (libchimera   ${CMAKE_LD_LIBS}
                                    ${CMAKE_THREAD_LIBS_INIT}
                                    ${OPENSSL_LIBRARIES})

# install lib
set (LIBCHIMERA_VERSION_MAJOR 0)
set (LIBCHIMERA_VERSION_MINOR 7)
set (LIBCHIMERA_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBCHIMERA_VERSION_MAJOR}.${LIBCHIMERA_VERSION_MINOR}.${LIBCHIMERA_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBCHIMERA_VERSION_MAJOR})
set_target_properties(libchimera PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME chimera)
