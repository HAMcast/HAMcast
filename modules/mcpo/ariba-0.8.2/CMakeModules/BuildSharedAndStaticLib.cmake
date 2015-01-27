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

# Variables to set for this macro to work:
# ${lib_name}_SOURCES        - The source files the library consists of
# ${lib_name}_LINK_LIBRARIES - The libraries the library should be linked to
# ${lib_name}_VERSION        - The version of the library
#                              (used for the shared lib)
# ${lib_name}_SOVERSION      - The the SO version of the library
#                              (used for the shared lib)
#
# After calling this macro the targets ${lib_name} and
# ${${lib_name}_STATIC_TARGET} will be available


macro(build_shared_and_static_libs lib_name)
    
    add_library(${lib_name} ${${lib_name}_SOURCES})
    target_link_libraries(${lib_name} ${${lib_name}_LINK_LIBRARIES})
    
    # Library versioning
    set_target_properties(${lib_name} PROPERTIES
        VERSION ${${lib_name}_VERSION}
        SOVERSION ${${lib_name}_SOVERSION}
        )
    
    # If we built a shared library additionally compile a static one
    if(BUILD_SHARED_LIBS)
        set(${lib_name}_STATIC_TARGET ${lib_name}_static)
        add_library(${${lib_name}_STATIC_TARGET} STATIC ${${lib_name}_SOURCES})
        set_target_properties(${${lib_name}_STATIC_TARGET} PROPERTIES OUTPUT_NAME ${lib_name})
        target_link_libraries(${${lib_name}_STATIC_TARGET} ${${lib_name}_LINK_LIBRARIES})
    endif(BUILD_SHARED_LIBS)
    
endmacro(build_shared_and_static_libs lib_name)
