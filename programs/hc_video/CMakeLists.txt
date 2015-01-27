cmake_minimum_required(VERSION 2.6)
project (hamcast_video)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# search for libs
find_package (LibHamcast REQUIRED)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibAV REQUIRED)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( . ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS} /opt/local/include)

add_executable (hcvideo main.cpp)

target_link_libraries (hcvideo  ${HAMCAST_LIBRARY}
                                #/usr/lib/x86_64-linux-gnu/libavformat.so
                                #/usr/lib/x86_64-linux-gnu/libavcodec.so
                                #/usr/lib/x86_64-linux-gnu/libavutil.so
                                #/opt/local/lib/libavformat.dylib
                                #/opt/local/lib/libavcodec.dylib
                                #/opt/local/lib/libavutil.dylib
                                ${LIBAV_LIBRARIES}
                                ${Boost_DATE_TIME_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY}
                                ${Boost_FILESYSTEM_LIBRARY}
                                ${Boost_REGEX_LIBRARY}
                                ${Boost_THREAD_LIBRARY})
