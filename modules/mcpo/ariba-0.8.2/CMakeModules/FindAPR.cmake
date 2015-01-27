# - Find apr
# Find the Apache Portable Runtime Library headers and libraries.
#
# respects the BUILD_STATIC_BINS variable => searches for static library and its
# dependencies
#
#  APR_INCLUDE_DIRS     - where to find apr.h etc.
#  APRUTIL_INCLUDE_DIRS - where to find apu.h etc.
#  APR_LIBRARIES        - List of libraries when using apr.
#  APRUTIL_LIBRARIES    - List of libraries when using aprutil.
#  APR_FOUND            - True if apr found.
#  APRUTIL_FOUND        - True if aprutil found.


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

find_package(PkgConfig)
pkg_check_modules(PC_APR1 QUIET apr-1)
pkg_check_modules(PC_APRUTIL1 QUIET apr-util-1)


# Build static binaries?
if(BUILD_STATIC_BINS)
    set(STATIC_ "STATIC_")
endif(BUILD_STATIC_BINS)

set(APR_DEFINITIONS ${PC_APR1_${STATIC_}CFLAGS_OTHER})
set(APRUTIL_DEFINITIONS ${PC_APRUTIL1_${STATIC_}CFLAGS_OTHER})


# Look for the apr header files.
find_path(APR_INCLUDE_DIR NAMES apr.h
    HINTS
        ${PC_APR1_${STATIC_}INCLUDEDIR}
        ${PC_APR1_${STATIC_}INCLUDE_DIRS})
mark_as_advanced(APR_INCLUDE_DIR)
set(handle_standard_args_variables APR_INCLUDE_DIR)

# Look for the aprutil header files.
find_path(APRUTIL_INCLUDE_DIR NAMES apu.h
    HINTS
        ${PC_APRUTIL1_${STATIC_}INCLUDEDIR}
        ${PC_APRUTIL1_${STATIC_}INCLUDE_DIRS})
mark_as_advanced(APRUTIL_INCLUDE_DIR)
set(util_handle_standard_args_variables APRUTIL_INCLUDE_DIR)


# Look for the libraries
if(NOT PC_APR1_${STATIC_}LIBRARIES)
    set(PC_APR1_${STATIC_}LIBRARIES apr-1)
endif()

foreach(LIBRARY ${PC_APR1_${STATIC_}LIBRARIES})
    find_library(APR_${STATIC_}LIBRARY_${LIBRARY} NAMES ${LIBRARY}
        DOC "The path to the ${LIBRARY} library used by apr"
        HINTS
            ${PC_APR1_${STATIC_}LIBDIR}
            ${PC_APR1_LIBRARY_${STATIC_}DIRS}
        )
    mark_as_advanced(APR_${STATIC_}LIBRARY_${LIBRARY})
    list(APPEND handle_standard_args_variables
        "APR_${STATIC_}LIBRARY_${LIBRARY}"
        )
endforeach()

foreach(LIBRARY ${PC_APRUTIL1_${STATIC_}LIBRARIES})
    find_library(APRUTIL_${STATIC_}LIBRARY_${LIBRARY} NAMES ${LIBRARY}
        DOC "The path to the ${LIBRARY} library used by aprutil"
        HINTS
            ${PC_APRUTIL1_${STATIC_}LIBDIR}
            ${PC_APRUTIL1_LIBRARY_${STATIC_}DIRS}
        )
    mark_as_advanced(APRUTIL_${STATIC_}LIBRARY_${LIBRARY})
    list(APPEND util_handle_standard_args_variables
        "APRUTIL_${STATIC_}LIBRARY_${LIBRARY}"
        )
endforeach()

# handle the QUIETLY and REQUIRED arguments and set LOG4CXX_FOUND to TRUE
# if all listed variables are TRUE
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(APR DEFAULT_MSG
    ${handle_standard_args_variables}
    )

if(APR_FOUND)
    foreach(LIBRARY ${PC_APR1_${STATIC_}LIBRARIES})
        list(APPEND APR_LIBRARIES ${APR_${STATIC_}LIBRARY_${LIBRARY}})
    endforeach()
    
    # handle APRUTIL
    foreach(pred ${util_handle_standard_args_variables})
        if(NOT ${pred})
            set(APRUTIL_FOUND FALSE)
        endif()
    endforeach()
    
    if(NOT DEFINED APRUTIL_FOUND)
        set(APRUTIL_FOUND TRUE)
    endif(NOT DEFINED APRUTIL_FOUND)
    
    if(APRUTIL_FOUND)
        find_package(PackageMessage)
        
        foreach(LIBRARY ${PC_APRUTIL1_${STATIC_}LIBRARIES})
            set(APRUTIL_${STATIC_}LIBRARY_${LIBRARY}_FIND_QUIETLY ${APR_FIND_QUIETLY})
            find_package_message(APRUTIL_${STATIC_}LIBRARY_${LIBRARY}
                "Found ${LIBRARY}: ${APRUTIL_${STATIC_}LIBRARY_${LIBRARY}}"
                "[${APRUTIL_${STATIC_}LIBRARY_${LIBRARY}}]")
            list(APPEND APRUTIL_LIBRARIES ${APRUTIL_${STATIC_}LIBRARY_${LIBRARY}})
        endforeach()
    endif()
endif()
