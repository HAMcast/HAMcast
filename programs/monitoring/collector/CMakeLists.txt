cmake_minimum_required(VERSION 2.6)
project (monitoring_collector)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set (MONITORING_UTILS_PATH ../utils)

set (COLLECTOR_SOURCE async_tcp_server.cpp collector.cpp 
                    ${MONITORING_UTILS_PATH}/group.cpp 
                    ${MONITORING_UTILS_PATH}/http_message.cpp 
                    ${MONITORING_UTILS_PATH}/interface.cpp
                    ${MONITORING_UTILS_PATH}/method_caller.hpp
                    multicast_module.cpp
                    function_wrapper.cpp
                    ${MONITORING_UTILS_PATH}/node.cpp 
                    tcp_session.cpp main.cpp)

# search for libs
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${MONITORING_UTILS_PATH} ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} )

add_executable (mcollector ${COLLECTOR_SOURCE})

target_link_libraries (mcollector  ${HAMCAST_LIBRARY} 
                                ${Boost_DATE_TIME_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY}
                                ${Boost_REGEX_LIBRARY}
                                ${Boost_THREAD_LIBRARY})
