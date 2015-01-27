cmake_minimum_required(VERSION 2.6)
project (performance-test)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
#    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
### exchange these lines if compiling on planetlab or other x86 systems
    set (CMAKE_CXX_FLAGS "-DHC_ENABLE_LOGGING -Wextra -Wall -pedantic -march=i486")
else ()
 # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
### exchange these lines if compiling on planetlab or other x86 systems
   set (CMAKE_CXX_FLAGS "-O2 -march=i486")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (PERFORMANCETEST_SRC  
                        src/main.cpp
                        include/main.hpp
	)



# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)

include_directories (./include ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

add_executable(performancetest ${PERFORMANCETEST_SRC})

target_link_libraries (performancetest ${CMAKE_LD_LIBS}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY})

