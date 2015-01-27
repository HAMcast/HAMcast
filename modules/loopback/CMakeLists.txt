cmake_minimum_required(VERSION 2.6)
project (hamcast_libvoid_module)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)

include_directories (. ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

add_library(libloopbackmodule SHARED loopback.cpp)

target_link_libraries (libloopbackmodule ${CMAKE_LD_LIBS}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY})

# install lib
set (LIBLOOP_MODULE_VERSION_MAJOR 0)
set (LIBLOOP_MODULE_VERSION_MINOR 7)
set (LIBLOOP_MODULE_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBLOOP_MODULE_VERSION_MAJOR}.${LIBLOOP_MODULE_VERSION_MINOR}.${LIBLOOP_MODULE_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBLOOP_MODULE_VERSION_MAJOR})
set_target_properties(libloopbackmodule PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME loopbackmodule)
