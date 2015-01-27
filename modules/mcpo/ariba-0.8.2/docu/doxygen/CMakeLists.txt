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

option(DOCUMENTATION_GENERATE_GRAPHICS
    "Whether graphics should be generated for the documentation. 
This might take a long time and quite some amount of space"
    OFF
    )

if(NOT DOCUMENTATION_GENERATE_GRAPHICS)
    set(DOXYGEN_SKIP_DOT TRUE)
endif(NOT DOCUMENTATION_GENERATE_GRAPHICS)

find_package(Doxygen)

if(DOXYGEN_FOUND)
    # Generate the Doxyfile
    configure_file(
        Doxyfile.in
        Doxyfile
        @ONLY
        )
    
    file(GLOB_RECURSE doxygen_SOURCES
        "${ariba_SOURCE_DIR}/source/*.c"
        "${ariba_SOURCE_DIR}/source/*.cc"
        "${ariba_SOURCE_DIR}/source/*.cxx"
        "${ariba_SOURCE_DIR}/source/*.cpp"
        "${ariba_SOURCE_DIR}/source/*.c++"
        "${ariba_SOURCE_DIR}/source/*.h"
        "${ariba_SOURCE_DIR}/source/*.hh"
        "${ariba_SOURCE_DIR}/source/*.hxx"
        "${ariba_SOURCE_DIR}/source/*.hpp"
        "${ariba_SOURCE_DIR}/source/*.h++"
        )
    
    add_custom_command(
        OUTPUT html/index.html
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
        MAIN_DEPENDENCY "${ariba_SOURCE_DIR}/source/ariba/ariba.h"
        DEPENDS
            ${doxygen_SOURCES}
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        COMMENT "Building Doxygen documentation"
        )
    
    
    option(ALWAYS_BUILD_DOCUMENTATION
        "Always build the documentation when compiling the project? If OFF the
documentation is only built when building the target 'docu'"
        OFF)
    if(ALWAYS_BUILD_DOCUMENTATION)
        set(BUILD_DOCU_ALL "ALL")
    endif()
    
    add_custom_target(docu ${BUILD_DOCU_ALL}
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/html/index.html"
            
        )
    
    
    install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/"
        DESTINATION share/doc/libariba-doc/html
        COMPONENT Documentation
        OPTIONAL
        )
    
    set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES html)
endif(DOXYGEN_FOUND)
