# - Find Avahi
# Find the Avahi library
#
#  AVAHI_INCLUDE_DIRS - where to find the header files
#  AVAHI_LIBRARIES    - List of libraries when using Avahi.
#  AVAHI_FOUND        - True if Avahi was found.


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

unset(handle_standard_args_variables)

foreach(COMPONENT ${Avahi_FIND_COMPONENTS})
    # Look for the header files.
    find_path(Avahi_${COMPONENT}_INCLUDE_DIR NAMES "avahi-${COMPONENT}/")
    mark_as_advanced(Avahi_${COMPONENT}_INCLUDE_DIR)
    
    # Look for the library.
    find_library(Avahi_${COMPONENT}_LIBRARY NAMES "avahi-${COMPONENT}"
        DOC "The path to the ${COMPONENT} component of the Avahi library"
        )
    mark_as_advanced(Avahi_${COMPONENT}_LIBRARY)
    
    list(APPEND handle_standard_args_variables
        "Avahi_${COMPONENT}_INCLUDE_DIR"
        "Avahi_${COMPONENT}_LIBRARY"
        )
    
endforeach(COMPONENT ${Avahi_FIND_COMPONENTS})


# handle the QUIETLY and REQUIRED arguments and set LIBBLUETOOTH_FOUND to TRUE
# if all listed variables are TRUE
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(Avahi DEFAULT_MSG
    ${handle_standard_args_variables}
    )


if(AVAHI_FOUND)
    foreach(COMPONENT ${Avahi_FIND_COMPONENTS})
        list(APPEND AVAHI_LIBRARIES "${Avahi_${COMPONENT}_LIBRARY}")
        list(APPEND AVAHI_INCLUDE_DIRS "${Avahi_${COMPONENT}_INCLUDE_DIR}")
    endforeach(COMPONENT ${Avahi_FIND_COMPONENTS})
    
    list(REMOVE_DUPLICATES AVAHI_INCLUDE_DIRS)
    
    
    # Add dependencies for client
    # TODO: handle dependencies of other parts
    list(FIND Avahi_FIND_COMPONENTS client Avahi_FIND_COMPONENT_CLIENT)
    if(Avahi_FIND_COMPONENT_CLIENT GREATER -1)
        find_package(DBus ${Avahi_FIND_REQUIRED} ${Avahi_FIND_QUIETLY})
        if(DBUS_FOUND)
            list(APPEND AVAHI_LIBRARIES ${DBUS_LIBRARIES})
        else()
            message(WARNING "Could not find the D-Bus interprocess messaging "
                "library which is a dependency of Avahi "
                "=> static linking might fail")
        endif()
    endif()
    
endif()
