cmake_minimum_required(VERSION 2.6)
project (hamcast_libscribe_module)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (LIBSCRIBE_INCLUDE include/)
set (LIBSCRIBE_SRC  src/scribe_module.cpp 
                    src/scribe_instance.cpp
                    src/scribe_group.cpp)

# search for libs
find_package (Threads REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)
find_package (LibChimera REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories (   ${LIBSCRIBE_INCLUDE} 
                        ${HAMCAST_INCLUDE}
                        ${CHIMERA_INCLUDE}
                        ${OPENSSL_INCLUDE_DIR} 
                        ${Boost_INCLUDE_DIRS})

add_library(libscribemodule SHARED ${LIBSCRIBE_SRC})

target_link_libraries (libscribemodule ${CMAKE_LD_LIBS}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY}
                            ${CHIMERA_LIBRARY}
                            ${OPENSSL_LIBRARIES})

# install lib
set (LIBSCRIBE_VERSION_MAJOR 0)
set (LIBSCRIBE_VERSION_MINOR 7)
set (LIBSCRIBE_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBSCRIBE_VERSION_MAJOR}.${LIBSCRIBE_VERSION_MINOR}.${LIBSCRIBE_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBSCRIBE_VERSION_MAJOR})
set_target_properties(libscribemodule PROPERTIES 
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME scribemodule)
