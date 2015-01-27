cmake_minimum_required(VERSION 2.6)
project (hamcast_make_hash)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# search for libs
find_package (LibChimera REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories ( . ${CHIMERA_INCLUDE} ${OPENSSL_INCLUDE_DIR})

add_executable(make_hash main.cpp)

target_link_libraries (make_hash ${CMAKE_LD_LIBS} ${CHIMERA_LIBRARY} ${OPENSSL_LIBRARIES})
