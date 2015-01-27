cmake_minimum_required(VERSION 2.6)
project (monitoring_daemon)

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
set (MONITORING_UTILS_PATH ../utils)

set (DAEMON_SOURCE  ${MONITORING_UTILS_PATH}/group.cpp 
                    ${MONITORING_UTILS_PATH}/http_message.cpp 
                    ${MONITORING_UTILS_PATH}/interface.cpp
                    ${MONITORING_UTILS_PATH}/method_caller.hpp 
                    multicast_module.cpp
                    function_wrapper.cpp
                    ${MONITORING_UTILS_PATH}/node.cpp 
                    tcp_client_connection.cpp main.cpp)

# search for libs
find_package (Threads)
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${MONITORING_UTILS_PATH} ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} )

add_executable (mdaemon ${DAEMON_SOURCE})

target_link_libraries (mdaemon  ${CMAKE_THREAD_LIBS_INIT}
                                ${HAMCAST_LIBRARY} 
                                ${Boost_DATE_TIME_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY}
                                ${Boost_REGEX_LIBRARY}
                                ${Boost_THREAD_LIBRARY})
