cmake_minimum_required(VERSION 2.6)
project (hamcast_libip_module)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

find_package (Threads)
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)

if (ENABLE_DISCOVERY STREQUAL true)
    find_package (PCAP REQUIRED)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDISCOVERY")
    set (LIBIP_INCLUDE include/ include/discovery)
    set (LIBIP_SRC  src/ip_socket.cpp src/ip_instance.cpp src/ip_module.cpp
                    src/discovery/addr_storage.cpp src/discovery/dis_tank.cpp
                    src/discovery/igmp_dis.cpp src/discovery/mc_socket.cpp
                    src/discovery/mld_dis.cpp src/discovery/pcap_sniffer.cpp
                    src/discovery/service_discovery.cpp src/discovery/basic_dis.cpp
                    src/discovery/if_prop.cpp src/discovery/igmp_light_dis.cpp
                    src/discovery/mc_tables.cpp src/discovery/mld_light_dis.cpp
                    src/discovery/mroute_socket.cpp src/discovery/raw_socket.cpp)

    include_directories (${LIBIP_INCLUDE} ${PCAP_INCLUDE_DIRS} ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

    add_library(libipmodule SHARED ${LIBIP_SRC})

    target_link_libraries (libipmodule ${CMAKE_LD_LIBS}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY}
                            ${PCAP_LIBRARIES}
                            ${OPENSSL_LIBRARIES})

else (ENABLE_DISCOVERY STREQUAL true)
    set (LIBIP_INCLUDE include/)
    set (LIBIP_SRC  src/ip_socket.cpp src/ip_instance.cpp src/ip_module.cpp)

    include_directories (${LIBIP_INCLUDE} ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

    add_library(libipmodule SHARED ${LIBIP_SRC})

    target_link_libraries (libipmodule ${CMAKE_LD_LIBS}
                            ${CMAKE_THREAD_LIBS_INIT}
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY}
                            ${OPENSSL_LIBRARIES})

endif (ENABLE_DISCOVERY STREQUAL true)

# install lib
set (LIBIP_VERSION_MAJOR 0)
set (LIBIP_VERSION_MINOR 7)
set (LIBIP_VERSION_PATCH 0)
set (LIBRARY_VERSION ${LIBIP_VERSION_MAJOR}.${LIBIP_VERSION_MINOR}.${LIBIP_VERSION_PATCH})
set (LIBRARY_SOVERSION ${LIBIP_VERSION_MAJOR})
set_target_properties(libipmodule PROPERTIES 
                        SOVERSION ${LIBRARY_SOVERSION} 
                        VERSION ${LIBRARY_VERSION} 
                        OUTPUT_NAME ipmodule)
