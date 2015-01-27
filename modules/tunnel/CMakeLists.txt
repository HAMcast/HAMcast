cmake_minimum_required(VERSION 2.6)
project (hamcast_libtunnel_module)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (LIBTUNNELMODULE_SRC    src/tunnel_message.cpp
                            src/tunnel.cpp
                            src/module.cpp)

# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)

include_directories (. src/ ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

add_library(libtunnelmodule SHARED ${LIBTUNNELMODULE_SRC})

target_link_libraries (libtunnelmodule ${CMAKE_LD_LIBS}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY})

# install lib
set (LIBTUNNELMODULE_VERSION_MAJOR 0)
set (LIBTUNNELMODULE_VERSION_MINOR 7)
set (LIBTUNNELMODULE_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBTUNNELMODULE_VERSION_MAJOR}.${LIBTUNNELMODULE_VERSION_MINOR}.${LIBTUNNELMODULE_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBTUNNELMODULE_VERSION_MAJOR})
set_target_properties(libtunnelmodule PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME tunnelmodule)
