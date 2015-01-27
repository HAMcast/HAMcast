# [License]
# The Ariba-Underlay Copyright
#
# Copyright (c) 2008-2012, Institute of Telematics, Universität Karlsruhe (TH)
#
# Institute of Telematics
# Universität Karlsruhe (TH)
# Zirkel 2, 76128 Karlsruhe
# Germany
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OF TELEMATICS OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation
# are those of the authors and should not be interpreted as representing
# official policies, either expressed or implied, of the Institute of
# Telematics.
# [License]

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(libariba)

list(APPEND CMAKE_MODULE_PATH "${libariba_SOURCE_DIR}/../../CMakeModules")


######################################################
### Increment this whenever the interface changes! ###
######################################################
set(ariba_SOVERSION 1)
######################################################


# Find Boost
find_package(BoostAdditionalVersions QUIET)
if(NOT DEFINED BoostAdditionalVersions_FOUND)
    message(WARNING "Could not find FindBoostAdditionalVersions.cmake. "
        "This might cause the Boost detection to fail")
endif()
find_package(Boost 1.42.0 REQUIRED COMPONENTS system thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost. "
            "Please have a look at the Boost_* cache variables.")
endif(NOT Boost_FOUND)

mark_as_advanced(Boost_DIR)
include(BoostPthreadWorkaround)

include_directories(${Boost_INCLUDE_DIRS})
list(APPEND ariba_LINK_LIBRARIES ${Boost_LIBRARIES})


# Find gmp
find_package(GMP REQUIRED)
if(NOT GMP_FOUND)
    message(FATAL_ERROR "Could not find GMP. "
            "Please have a look at the GMP_* cache variables.")
endif(NOT GMP_FOUND)
include_directories(${GMP_INCLUDE_DIRS})
list(APPEND ariba_LINK_LIBRARIES ${GMP_LIBRARIES})


# Bluetooth support?
find_package(LibBluetooth)
if(NOT LIBBLUETOOTH_FOUND)
    message(STATUS "Couldn't find libbluetooth. "
        "Will build ariba withouth bluetooth support")
endif()
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_BLUETOOTH
    "Enable bluetooth support" #doc
    ON # default value
    "LIBBLUETOOTH_FOUND" # only show if this evaluates to true
    OFF # value if the condition is not met
    )
if(ENABLE_BLUETOOTH)
    set(HAVE_LIBBLUETOOTH TRUE)
    include_directories("${LIBBLUETOOTH_INCLUDE_DIR}")
    list(APPEND ariba_LINK_LIBRARIES ${LIBBLUETOOTH_LIBRARIES})
endif(ENABLE_BLUETOOTH)


# Avahi support?
find_package(Avahi COMPONENTS client common)
if(NOT AVAHI_FOUND)
    message(STATUS "Couldn't find Avahi. "
        "Will build ariba withouth multicast DNS support")
endif()
cmake_dependent_option(ENABLE_AVAHI
    "Enable Avahi (multicast DNS) support" #doc
    ON # default value
    "AVAHI_FOUND" # only show if this evaluates to true
    OFF # value if the condition is not met
    )
if(ENABLE_AVAHI)
    set(HAVE_AVAHI TRUE)
    include_directories(${AVAHI_INCLUDE_DIRS})
    list(APPEND ariba_LINK_LIBRARIES ${AVAHI_LIBRARIES})
endif(ENABLE_AVAHI)


# Log4Cxx support?
find_package(Log4Cxx)
if(NOT LOG4CXX_FOUND)
    message(STATUS "Couldn't find Log4Cxx. "
        "Will build ariba withouth extended logging support")
endif()
cmake_dependent_option(ENABLE_LOG4CXX
    "Enable Log4Cxx (extended logging) support" #doc
    ON # default value
    "LOG4CXX_FOUND" # only show if this evaluates to true
    OFF # value if the condition is not met
    )
if(ENABLE_LOG4CXX)
    set(HAVE_LOG4CXX TRUE)
    include_directories(${LOG4CXX_INCLUDE_DIRS})
    list(APPEND ariba_LINK_LIBRARIES ${LOG4CXX_LIBRARIES})
endif(ENABLE_LOG4CXX)



set(add_headers_VAR libariba_HEADERS)
set(add_sources_VAR libariba_SOURCES)

# Voodoo to collect the source files from the subdirectories
function(add_subdir_sources_helper subdir)
    set(CURRENT_SOURCE_DIR "${CURRENT_SOURCE_DIR}${subdir}/")
    add_subdirectory(${subdir})
    set(${add_headers_VAR} ${${add_headers_VAR}} PARENT_SCOPE)
    set(${add_sources_VAR} ${${add_sources_VAR}} PARENT_SCOPE)
endfunction(add_subdir_sources_helper subdir)

macro(add_subdir_sources subdirs)
    foreach(subdir ${subdirs} ${ARGN})
        add_subdir_sources_helper(${subdir})
    endforeach(subdir ${subdirs} ${ARGN})
    set(${add_headers_VAR} ${${add_headers_VAR}} PARENT_SCOPE)
    set(${add_sources_VAR} ${${add_sources_VAR}} PARENT_SCOPE)
endmacro(add_subdir_sources subdirs)

macro(add_headers sources)
    foreach(source ${sources} ${ARGN})
        list(APPEND ${add_headers_VAR} "${CURRENT_SOURCE_DIR}${source}")
    endforeach(source ${sources} ${ARGN})
    set(${add_headers_VAR} ${${add_headers_VAR}} PARENT_SCOPE)
endmacro(add_headers sources)

macro(add_sources sources)
    foreach(source ${sources} ${ARGN})
        list(APPEND ${add_sources_VAR} "${CURRENT_SOURCE_DIR}${source}")
    endforeach(source ${sources} ${ARGN})
    set(${add_sources_VAR} ${${add_sources_VAR}} PARENT_SCOPE)
endmacro(add_sources sources)


# Headers to be exported
set(libariba_HEADERS
    ariba.h
    AribaModule.h
    CommunicationListener.h
    DataMessage.h
    Identifiers.h
    LinkProperties.h
    Message.h
    Module.h
    Name.h
    Node.h
    NodeListener.h
    SideportListener.h
    SpoVNetProperties.h
    )

set(libariba_SOURCES
    AribaModule.cpp
    CommunicationListener.cpp
    DataMessage.cpp
    Identifiers.cpp
    LinkProperties.cpp
    Module.cpp
    Name.cpp
    Node.cpp
    NodeListener.cpp
    SideportListener.cpp
    SpoVNetProperties.cpp
    )

# Can't use add_subdir_sources() here because we want it in this scope not
# in our parent's scope
add_subdir_sources_helper(communication)
add_subdir_sources_helper(overlay)
add_subdir_sources_helper(utility)


if(CMAKE_BUILD_TYPE)
    if(${CMAKE_BUILD_TYPE} STREQUAL Debug)
        set(DEBUG ON)
    endif()
endif()

option(HAVE_MAEMO
    "Whether we compile for the maemo platform" #TODO: maybe detect automatically
    OFF
    )

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

# Configure the template file
configure_file(
    config.h.in
    config.h
    )
include_directories("${CMAKE_CURRENT_BINARY_DIR}/..")
list(APPEND libariba_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/config.h")

# Add the parent directory to the includes because we reference headers with
# "ariba/path/to/file" all the time
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")


# Compile the actual library

include(BuildSharedAndStaticLib)

set(ariba_SOURCES ${libariba_SOURCES} ${libariba_HEADERS})
# ariba_LINK_LIBRARIES, ariba_VERSION and ariba_SOVERSION already defined

build_shared_and_static_libs(ariba)


# Installation stuff
install(TARGETS ariba ${ariba_STATIC_TARGET} EXPORT ariba-targets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

install(FILES ${libariba_HEADERS}
    DESTINATION include/ariba
    COMPONENT Development
    )

# Make libariba usable from build tree
export(TARGETS ariba ${ariba_STATIC_TARGET} FILE ariba-exports.cmake)

if(ariba_BINARY_DIR)
    export(TARGETS ariba ${ariba_STATIC_TARGET}
        FILE "${ariba_BINARY_DIR}/ariba-exports.cmake")
endif(ariba_BINARY_DIR)
