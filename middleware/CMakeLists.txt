cmake_minimum_required(VERSION 2.6)
project (hamcast_middleware)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
### exchange these lines if compiling on planetlab or other x86 systems
#    set (CMAKE_CXX_FLAGS "-DHC_ENABLE_LOGGING -Wextra -Wall -pedantic -march=i486")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
### exchange these lines if compiling on planetlab or other x86 systems
#    set (CMAKE_CXX_FLAGS "-O2 -march=i486")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

set (MIDDLEWARE_SRC connection.cpp middleware.cpp
                    rw_spinlock.cpp send_job.cpp
                    session.cpp socket_proxy.cpp
                    tech_module.cpp main.cpp)

# search for libs
if (NOT HAMCAST_LIBRARY)
    find_package (LibHamcast REQUIRED)
endif (NOT HAMCAST_LIBRARY)

find_package (Threads)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} )

add_executable (middleware ${MIDDLEWARE_SRC})

target_link_libraries (middleware ${CMAKE_DL_LIBS}
                                  ${CMAKE_THREAD_LIBS_INIT} 
                                  ${HAMCAST_LIBRARY} 
                                  ${Boost_DATE_TIME_LIBRARY}
                                  ${Boost_SYSTEM_LIBRARY}
                                  ${Boost_FILESYSTEM_LIBRARY}
                                  ${Boost_REGEX_LIBRARY}
                                  ${Boost_THREAD_LIBRARY})
