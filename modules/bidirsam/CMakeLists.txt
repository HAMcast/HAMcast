cmake_minimum_required(VERSION 2.6)
project (hamcast_libbidirsam_module)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (LIBBIDIRSAM_INCLUDE include/)
set (LIBBIDIRSAM_SRC src/bidirsam_module.cpp 
		     src/bidirsam_mft.cpp 
		     src/bidirsam_instance.cpp)

set (LIBBIDIRSAM_HEADERS include/bidirsam.hpp include/bidirsam_instance.hpp include/bidirsam_mft.hpp
)
# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)
find_package (LibChimera REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories (${LIBBIDIRSAM_INCLUDE} ${HAMCAST_INCLUDE} ${CHIMERA_INCLUDE}
                     ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

add_library(libbidirsammodule SHARED ${LIBBIDIRSAM_SRC})

target_link_libraries (libbidirsammodule ${CMAKE_LD_LIBS}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY}
			    ${CHIMERA_LIBRARY}
                            ${OPENSSL_LIBRARIES})

# install lib
set (LIBBIDIRSAM_VERSION_MAJOR 0)
set (LIBBIDIRSAM_VERSION_MINOR 7)
set (LIBBIDIRSAM_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBBIDIRSAM_VERSION_MAJOR}.${LIBBIDIRSAM_VERSION_MINOR}.${LIBBIDIRSAM_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBBIDIRSAM_VERSION_MAJOR})
set_target_properties(libbidirsammodule PROPERTIES 
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME bidirsammodule)
