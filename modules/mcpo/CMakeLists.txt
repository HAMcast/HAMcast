cmake_minimum_required(VERSION 2.6)
project (hamcast_mcpo_module)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
### exchange these lines if compiling on planetlab or other x86 systems
 #   set (CMAKE_CXX_FLAGS "-DHC_ENABLE_LOGGING -Wextra -Wall -pedantic -march=i486")
else ()
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 ")
### exchange these lines if compiling on planetlab or other x86 systems
#    set (CMAKE_CXX_FLAGS "-O2 -march=i486")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set (LIBMCPOMODULE_SRC  
                        src/mcpo_connection.cpp
			src/mcpomodule.cpp
			src/receiver.cpp
                        include/mcpo_connection.h
			include/mcpomodule.h
			include/receiver.h
			include/instance_handlers.h
	)



# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)
find_package (LibAriba REQUIRED)
find_package (LibMcpo REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories (./include ${HAMCAST_INCLUDE} ${ARIBA_INCLUDE} ${MCPO_INCLUDE} ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})

add_library(libmcpomodule SHARED ${LIBMCPOMODULE_SRC})

target_link_libraries (libmcpomodule ${CMAKE_LD_LIBS}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${ARIBA_LIBRARY}
                            ${ARIBA_DHT_LIBRARY}
                            ${MCPO_LIBRARY}
                            ${HAMCAST_LIBRARY}
                            ${OPENSSL_LIBRARIES})

# install lib
set (LIBMCPOMODULE_VERSION_MAJOR 0)
set (LIBMCPOMODULE_VERSION_MINOR 4)
set (LIBMCPOMODULE_VERSION_PATCH 1)
set(LIBRARY_VERSION ${LIBMCPOMODULE_VERSION_MAJOR}.${LIBMCPOMODULE_VERSION_MINOR}.${LIBMCPOMODULE_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBMCPOMODULE_VERSION_MAJOR})
set_target_properties(libmcpomodule PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME mcpomodule)
