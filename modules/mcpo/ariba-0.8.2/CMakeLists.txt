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
project(ariba CXX)

# Version
set(ariba_VERSION_MAJOR 0)
set(ariba_VERSION_MINOR 8)
set(ariba_VERSION_PATCH 2)
set(ariba_VERSION ${ariba_VERSION_MAJOR}.${ariba_VERSION_MINOR}.${ariba_VERSION_PATCH})

message(STATUS "Configuring ariba version ${ariba_VERSION}")


list(APPEND CMAKE_MODULE_PATH "${ariba_SOURCE_DIR}/CMakeModules")

# Provide some choices for CMAKE_BUILD_TYPE
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "" Release Debug RelWithDebInfo MinSizeRel)

# Explicitly add BUILD_SHARED_LIBS to the user interface and default to on
option(BUILD_SHARED_LIBS "Whether to build libraries shared or static" ON)
mark_as_advanced(BUILD_SHARED_LIBS)



# Option to produce statically linked binaries
option(BUILD_STATIC_BINS
    "Whether to link executables statically.
If selected overrrides BUILD_SHARED_LIBS.
This option has to be set BEFORE the initial creation of the CMake cache
(this is only possible by using the non-interactive command line interface
`cmake ../ -DBUILD_STATIC_BINS=ON`)
Otherwise the library paths will not be rediscovered."
    OFF
    )

if(DEFINED BUILD_STATIC_BINS_PREVIOUS_VALUE AND
        ((BUILD_STATIC_BINS_PREVIOUS_VALUE AND NOT BUILD_STATIC_BINS) OR
        (NOT BUILD_STATIC_BINS_PREVIOUS_VALUE AND BUILD_STATIC_BINS)))
    message(FATAL_ERROR
        "You can't set BUILD_STATIC_BINS to a different value after the "
        "CMake cache has been built because then the library paths will not "
        "be rediscovered. "
        "Setting this before cache creation is only possible by using the "
        "non-interactive command line interface: "
        "`cmake ../ -DBUILD_STATIC_BINS=ON`"
        )
endif()
set(BUILD_STATIC_BINS_PREVIOUS_VALUE "${BUILD_STATIC_BINS}" CACHE INTERNAL
    "Internal value to detect whether the BUILD_STATIC_BINS has been previously
    set to a different value"
    )

if(BUILD_STATIC_BINS)
    set(BUILD_SHARED_LIBS OFF)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    # Give priority to .a files (or equivalent)
    set(CMAKE_FIND_LIBRARY_SUFFIXES 
            ${CMAKE_STATIC_LIBRARY_SUFFIX} ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif(BUILD_STATIC_BINS)
# End of option to produce statically linked binaries


add_subdirectory(source)
add_subdirectory(sample)
add_subdirectory(docu)

# Install package information
install(EXPORT ariba-targets
    DESTINATION lib/ariba
    COMPONENT Development
    )

configure_file(
    pkg/ariba-config.cmake.in
    pkg/ariba-config.cmake
    @ONLY
    )

configure_file(
    pkg/ariba-config-local.cmake.in
    ariba-config.cmake
    @ONLY
    )

configure_file(
    pkg/ariba-config-version.cmake.in
    ariba-config-version.cmake
    @ONLY
    )

install(
    FILES
        "${ariba_BINARY_DIR}/pkg/ariba-config.cmake"
        "${ariba_BINARY_DIR}/ariba-config-version.cmake"
    DESTINATION lib/ariba
    COMPONENT Development
    )



# Packaging
set(CPACK_PACKAGE_VERSION_MAJOR
    "${ariba_VERSION_MAJOR}"
    )
set(CPACK_PACKAGE_VERSION_MINOR
    "${ariba_VERSION_MINOR}"
    )
set(CPACK_PACKAGE_VERSION_PATCH
    "${ariba_VERSION_PATCH}"
    )

set(CPACK_SYSTEM_NAME
    "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}"
    )

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Ariba P2P library"
    )
#TODO: set(CPACK_PACKAGE_DESCRIPTION_FILE)
#TODO: set(CPACK_RESOURCE_FILE_README)
#TODO: set(CPACK_RESOURCE_FILE_WELCOME)

set(CPACK_PACKAGE_VENDOR
    "Institute of Telematics, Universität Karlsruhe (TH)"
    )

set(CPACK_SOURCE_PACKAGE_FILE_NAME "ariba-${ariba_VERSION}")

include(CPack)
