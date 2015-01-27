cmake_minimum_required(VERSION 2.6)
project (hamcast_dimg)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# search for libs
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} )

add_executable (dimg main.cpp)

target_link_libraries (dimg     ${HAMCAST_LIBRARY} 
                                ${Boost_DATE_TIME_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY}
                                ${Boost_REGEX_LIBRARY}
                                ${Boost_THREAD_LIBRARY})
