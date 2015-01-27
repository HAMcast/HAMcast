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

######################################################
### Increment this whenever the interface changes! ###
######################################################
set(ariba_dht_SOVERSION 1)
######################################################

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


include(BuildSharedAndStaticLib)

set(ariba_dht_HEADERS
    Dht.h
    DhtAnswerInterface.h
    )

set(ariba_dht_SOURCES
    Dht.cpp
    messages/DhtMessage.cpp
    messages/DhtMessage.h
    ${ariba_dht_HEADERS}
    )

set(ariba_dht_LINK_LIBRARIES ariba ${Boost_LIBRARIES})
set(ariba_dht_VERSION ${ariba_VERSION})
# ariba_dht_SOVERSION already defined above

build_shared_and_static_libs(ariba_dht)


# Installation stuff
install(TARGETS ariba_dht ${ariba_dht_STATIC_TARGET} EXPORT ariba-targets
    LIBRARY DESTINATION lib COMPONENT Runtime
    ARCHIVE DESTINATION lib COMPONENT Development
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

install(FILES ${ariba_dht_HEADERS}
    DESTINATION include/ariba_dht
    COMPONENT Development
    )

# Make libariba_dht usable from build tree.
export(TARGETS ariba_dht ${ariba_dht_STATIC_TARGET}
    APPEND FILE "${ariba_BINARY_DIR}/ariba-exports.cmake"
    )
