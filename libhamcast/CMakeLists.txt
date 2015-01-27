cmake_minimum_required(VERSION 2.4)
project (hamcast_libhamcast)

# Set up environment paths to cmake modules and libhamcast
set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_ENABLE_LOGGING -g -Wextra -Wall -pedantic")
### exchange these lines if compiling on planetlab or other x86 systems
#    set (CMAKE_CXX_FLAGS "-DHC_ENABLE_LOGGING -Wextra -Wall -pedantic -march=i486")
else ()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
### exchange these lines if compiling on planetlab or other x86 systems
#    set (CMAKE_CXX_FLAGS "-O2 -march=i486")
endif (CMAKE_BUILD_TYPE STREQUAL Debug)

# search for libs
find_package (Boost COMPONENTS date_time filesystem regex system thread REQUIRED)

set (LIBHAMCAST_SRC 
     src/async_multicast_socket.cpp
     src/channel.cpp src/client_channel.cpp
     src/const_buffer.cpp src/deserializer.cpp
     src/exception.cpp src/hamcast.cpp
     src/hamcast_logging.cpp src/interface_property.cpp
     src/ipc_api.cpp src/membership_event.cpp
     src/message.cpp src/middleware_configuration.cpp
     src/multicast_packet.cpp src/multicast_socket.cpp
     src/read_buffer.cpp src/ref_counted.cpp
     src/serializer.cpp src/sink.cpp
     src/serialization.cpp src/socket_io.cpp
     src/uri.cpp src/fake_client_channel.cpp
     src/client_channel_msg.cpp src/config_map.cpp)

if (HAMCAST_LITE) 
    message("-- Use fake middleware")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHC_USE_FAKE_MIDDLEWARE" )
    find_package (LibChimera REQUIRED)
    find_package (OpenSSL REQUIRED)
    set (LIBHAMCAST_SRC
         ${LIBHAMCAST_SRC}
         ../modules/ipm/src/ip_instance.cpp
         ../modules/ipm/src/ip_socket.cpp
         ../modules/scribe/src/scribe_group.cpp
         ../modules/scribe/src/scribe_instance.cpp
         ../modules/void/void.cpp
         ../modules/tunnel/src/tunnel.cpp
         ../modules/tunnel/src/tunnel_message.cpp)
    include_directories ( . ${Boost_INCLUDE_DIRS}
                         ${OPENSSL_INCLUDE_DIR} 
                         ${CMAKE_CURRENT_SOURCE_DIR}/../modules/ipm/include/
                         ${CMAKE_CURRENT_SOURCE_DIR}/../utils/chimera/include/
                         ${CMAKE_CURRENT_SOURCE_DIR}/../modules/scribe/include/
                         ${CMAKE_CURRENT_SOURCE_DIR}/../modules/void/
                         ${CMAKE_CURRENT_SOURCE_DIR}/../modules/tunnel/src/ )
    link_directories ( ${Boost_LIBRARY_DIRS} )

    add_library(libhamcast SHARED ${LIBHAMCAST_SRC})

    target_link_libraries (libhamcast ${CMAKE_LD_LIBS}
                                    ${Boost_DATE_TIME_LIBRARY}
                                    ${Boost_SYSTEM_LIBRARY}
                                    ${Boost_FILESYSTEM_LIBRARY}
                                    ${Boost_REGEX_LIBRARY}
                                    ${Boost_THREAD_LIBRARY}
                                    ${CHIMERA_LIBRARY}
                                    ${OPENSSL_LIBRARIES})

else (HAMCAST_LITE)
    include_directories ( . ${Boost_INCLUDE_DIRS} )

    link_directories ( ${Boost_LIBRARY_DIRS} )

    add_library(libhamcast SHARED ${LIBHAMCAST_SRC})

    target_link_libraries (libhamcast ${CMAKE_LD_LIBS}
                                  ${Boost_DATE_TIME_LIBRARY}
                                  ${Boost_SYSTEM_LIBRARY}
                                  ${Boost_FILESYSTEM_LIBRARY}
                                  ${Boost_REGEX_LIBRARY}
                                  ${Boost_THREAD_LIBRARY})

endif (HAMCAST_LITE)

# install lib
set(LIBHAMCAST_VERSION_MAJOR 0)
set(LIBHAMCAST_VERSION_MINOR 7)
set(LIBHAMCAST_VERSION_PATCH 0)
set(LIBRARY_VERSION ${LIBHAMCAST_VERSION_MAJOR}.${LIBHAMCAST_VERSION_MINOR}.${LIBHAMCAST_VERSION_PATCH})
set(LIBRARY_SOVERSION ${LIBHAMCAST_VERSION_MAJOR})
set_target_properties(libhamcast PROPERTIES SOVERSION ${LIBRARY_SOVERSION} VERSION ${LIBRARY_VERSION} OUTPUT_NAME hamcast)
install(TARGETS libhamcast LIBRARY DESTINATION lib)

# install includes
install(DIRECTORY hamcast/ DESTINATION include/hamcast
    FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY hamcast/ DESTINATION include/hamcast
    FILES_MATCHING PATTERN "*.h")
# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P
${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
