cmake_minimum_required(VERSION 2.6)
project (hamcast_libhamcast)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

#set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set (LIBIP_MODULE_INCLUDE src/ src/hcDiscovery)

if (ENABLE_DISCOVERY STREQUAL true)
    set (LIBIP_MODULE_SRC   src/utils.cpp src/ip_module.cpp
                            src/ip_utils.cpp src/native_socket.cpp
                            src/hcDiscovery/service_discovery.cpp
                            src/hcDiscovery/dis_tank.cpp
                            src/hcDiscovery/DiscoveryTools/basic_dis.cpp
                            src/hcDiscovery/DiscoveryTools/igmp_dis.cpp
                            src/hcDiscovery/DiscoveryTools/mld_dis.cpp
                            src/hcDiscovery/DiscoveryTools/mrinfo.cpp
                            src/hcDiscovery/DiscoveryTools/pim_v1_dis.cpp
                            src/hcDiscovery/DiscoveryTools/pim_v2_dis.cpp
                            src/hcDiscovery/DiscoveryTools/igmp_light_dis.cpp
                            src/hcDiscovery/DiscoveryTools/mld_light_dis.cpp
                            src/hcDiscovery/Inet_Stuff/if_prop.cpp
                            src/hcDiscovery/Inet_Stuff/pcap_sniffer.cpp
                            src/hcDiscovery/Inet_Stuff/mc_socket.cpp
                            src/hcDiscovery/Inet_Stuff/raw_socket.cpp
                            src/hcDiscovery/Inet_Stuff/addr_storage.cpp
                            src/hcDiscovery/Inet_Stuff/mroute_socket.cpp
                            src/hcDiscovery/Inet_Stuff/mc_tables.cpp
                            src/hcDiscovery/Inet_Stuff/Protocol_Infos/test_output.cpp
                            src/hcDiscovery/dis_tank.hpp
                            src/hcDiscovery/service_discovery.hpp
                            src/hcDiscovery/DiscoveryTools/basic_dis.hpp
                            src/hcDiscovery/DiscoveryTools/igmp_dis.hpp
                            src/hcDiscovery/DiscoveryTools/mld_dis.hpp
                            src/hcDiscovery/DiscoveryTools/mrinfo.hpp
                            src/hcDiscovery/DiscoveryTools/pim_v1_dis.hpp
                            src/hcDiscovery/DiscoveryTools/pim_v2_dis.hpp
                            src/hcDiscovery/DiscoveryTools/igmp_light_dis.hpp
                            src/hcDiscovery/DiscoveryTools/mld_light_dis.hpp
                            src/hcDiscovery/Inet_Stuff/if_prop.hpp
                            src/hcDiscovery/Inet_Stuff/mc_socket.hpp
                            src/hcDiscovery/Inet_Stuff/pcap_sniffer.hpp
                            src/hcDiscovery/Inet_Stuff/raw_socket.hpp
                            src/hcDiscovery/Inet_Stuff/addr_storage.hpp
                            src/hcDiscovery/Inet_Stuff/mroute_socket.hpp
                            src/hcDiscovery/Inet_Stuff/mc_tables.hpp
                            src/hcDiscovery/Inet_Stuff/protocol_Infos/pim.h
                            src/hcDiscovery/Inet_Stuff/Protocol_Infos/test_output.hpp
                            src/hcDiscovery/Inet_Stuff/Protocol_Infos/mc_timers_values.hpp
                            src/hcDiscovery/Inet_Stuff/Protocol_Infos/ip_v6_tool.hpp)
else (ENABLE_DISCOVERY STREQUAL true)
    set (LIBIP_MODULE_SRC   src/utils.cpp src/ip_module.cpp
                            src/ip_utils.cpp src/native_socket.cpp)
endif(ENABLE_DISCOVERY STREQUAL true)

find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)
find_package (LibHamcast REQUIRED)

include_directories (. ${LIBIP_MODULE_INCLUDE} 
                       ${HAMCAST_INCLUDE} ${Boost_INCLUDE_DIRS})

add_library(libipv4module SHARED ${LIBIP_MODULE_SRC})

target_link_libraries (libipv4module
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY})

# install lib
set (LIBIP_MODULE_VERSION_MAJOR 0)
set (LIBIP_MODULE_VERSION_MINOR 4)
set (LIBIP_MODULE_VERSION_PATCH 1)
set (LIBRARY_VERSION ${LIBIP_MODULE_VERSION_MAJOR}.${LIBIP_MODULE_VERSION_MINOR}.${LIBIP_MODULE_VERSION_PATCH})
set (LIBRARY_SOVERSION ${LIBIP_MODULE_VERSION_MAJOR})
set_target_properties(libipv4module PROPERTIES
                        SOVERSION ${LIBRARY_SOVERSION}
                        VERSION ${LIBRARY_VERSION}
                        OUTPUT_NAME ipv4module)

# IPv6 support (optional)
if (ENABLE_IPV6)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIPV6")
    add_library(libipv6module SHARED ${LIBIP_MODULE_SRC})
    target_link_libraries (libipv6module
                            ${Boost_DATE_TIME_LIBRARY}
                            ${Boost_SYSTEM_LIBRARY}
                            ${Boost_FILESYSTEM_LIBRARY}
                            ${Boost_REGEX_LIBRARY}
                            ${Boost_THREAD_LIBRARY}
                            ${HAMCAST_LIBRARY})
    set_target_properties(libipv6module PROPERTIES
                            SOVERSION ${LIBRARY_SOVERSION}
                            VERSION ${LIBRARY_VERSION}
                            OUTPUT_NAME ipv6module)
endif(ENABLE_IPV6)
