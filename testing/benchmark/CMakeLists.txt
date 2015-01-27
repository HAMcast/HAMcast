cmake_minimum_required(VERSION 2.6)
project (hamcast_benchmark)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)
# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# search for libs
find_package (Threads)
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} )

add_executable (benchmark src/getusage.c  src/pidof.c src/main.cpp)

target_link_libraries (benchmark
                       ${CMAKE_THREAD_LIBS_INIT}
                       ${HAMCAST_LIBRARY} 
                       ${Boost_DATE_TIME_LIBRARY}
                       ${Boost_SYSTEM_LIBRARY}
                       ${Boost_FILESYSTEM_LIBRARY}
                       ${Boost_REGEX_LIBRARY}
                       ${Boost_THREAD_LIBRARY})
