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

# Find Boost
find_package(BoostAdditionalVersions QUIET)
if(NOT DEFINED BoostAdditionalVersions_FOUND)
    message(WARNING "Could not find FindBoostAdditionalVersions.cmake. "
        "This might cause the Boost detection to fail")
endif()
find_package(Boost 1.42.0 REQUIRED COMPONENTS system)
mark_as_advanced(Boost_DIR)

include_directories(${Boost_INCLUDE_DIRS})


# Include ariba header files
include_directories("${ariba_SOURCE_DIR}/source/" "${ariba_BINARY_DIR}/source/")
add_executable(pingpong
    PingPongMessage.cpp
    PingPongMessage.h
    PingPong.cpp
    PingPong.h
    main.cpp
    )

if(${BUILD_STATIC_BINS})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
    string(REPLACE "-rdynamic" "" 
        CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS
        "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}"
        )
    # Compile system libraries statically
    set_target_properties(pingpong PROPERTIES
            LINK_SEARCH_END_STATIC ON)
endif(${BUILD_STATIC_BINS})

target_link_libraries(pingpong ariba ${Boost_LIBRARIES})

# Installation stuff
install(TARGETS pingpong
    DESTINATION lib/ariba
    COMPONENT Documentation
    )
