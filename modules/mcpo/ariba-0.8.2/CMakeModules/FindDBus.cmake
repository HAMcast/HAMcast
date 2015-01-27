# - Find D-Bus
# Find the D-Bus inter-process messaging library
#
# respects the BUILD_STATIC_BINS variable => searches for static library and its
# dependencies
#
#  DBUS_INCLUDE_DIR  - where to find dbus/dbus.h etc.
#  DBUS_LIBRARIES    - List of libraries when using D-Bus.
#  DBUS_FOUND        - True if D-Bus was found.


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
pkg_check_modules(PC_DBUS QUIET dbus-1)

# Build static binaries?
if(BUILD_STATIC_BINS)
    set(STATIC_ "STATIC_")
endif(BUILD_STATIC_BINS)

set(DBUS_DEFINITIONS ${PC_DBUS_${STATIC_}CFLAGS_OTHER})


# Look for the header files.
find_path(DBUS_INCLUDE_DIR NAMES dbus/dbus.h
    HINTS
        ${PC_DBUS_${STATIC_}INCLUDEDIR}
        ${PC_DBUS_${STATIC_}INCLUDE_DIRS})
mark_as_advanced(DBUS_INCLUDE_DIR)
set(handle_standard_args_variables DBUS_INCLUDE_DIR)


# Look for the libraries
if(NOT PC_DBUS_${STATIC_}LIBRARIES)
    set(PC_DBUS_${STATIC_}LIBRARIES dbus-1)
endif()

foreach(LIBRARY ${PC_DBUS_${STATIC_}LIBRARIES})
    find_library(DBUS_${STATIC_}LIBRARY_${LIBRARY} NAMES ${LIBRARY}
        DOC "The path to the ${LIBRARY} library used by D-Bus"
        HINTS
            ${PC_DBUS_${STATIC_}LIBDIR}
            ${PC_DBUS_LIBRARY_${STATIC_}DIRS}
        )
    mark_as_advanced(DBUS_${STATIC_}LIBRARY_${LIBRARY})
    list(APPEND handle_standard_args_variables
        "DBUS_${STATIC_}LIBRARY_${LIBRARY}"
        )
endforeach()


# handle the QUIETLY and REQUIRED arguments and set DBUS_FOUND to TRUE
# if all listed variables are TRUE
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(DBus DEFAULT_MSG
    ${handle_standard_args_variables}
    )

if(DBUS_FOUND)
    foreach(LIBRARY ${PC_DBUS_${STATIC_}LIBRARIES})
        list(APPEND DBUS_LIBRARIES
            ${DBUS_${STATIC_}LIBRARY_${LIBRARY}}
            )
    endforeach()
endif()
