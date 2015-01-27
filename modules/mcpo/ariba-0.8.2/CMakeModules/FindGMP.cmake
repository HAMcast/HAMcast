# - Find gmp
# Find the GNU Multiple Precision Arithmetic Library headers and libraries.
#
#  GMP_INCLUDE_DIRS   - where to find gmp.h etc.
#  GMPXX_INCLUDE_DIRS - where to find gmpxx.h etc.
#  GMP_LIBRARIES      - List of libraries when using gmp.
#  GMPXX_LIBRARIES    - List of libraries when using the C++ wrapper of gmp.
#  GMP_FOUND          - True if gmp found.
#  GMPXX_FOUND        - True if C++ wrapper of gmp found.


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

# Look for the header files.
find_path(GMP_INCLUDE_DIR NAMES gmp.h)
mark_as_advanced(GMP_INCLUDE_DIR)

find_path(GMPXX_INCLUDE_DIR NAMES gmpxx.h)
mark_as_advanced(GMPXX_INCLUDE_DIR)


# Look for the library.
find_library(GMP_LIBRARY NAMES gmp
    DOC "The path to the GNU Multiple Precision Arithmetic Library"
    )
mark_as_advanced(GMP_LIBRARY)

find_library(GMPXX_LIBRARY NAMES gmpxx
    DOC "The path to the C++ wrapper of the GNU Multiple Precision Arithmetic Library"
    )
mark_as_advanced(GMPXX_LIBRARY)


# handle the QUIETLY and REQUIRED arguments and set GMP_FOUND to TRUE if 
# all listed variables are TRUE
find_package(PackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG GMP_LIBRARY GMP_INCLUDE_DIR)

if(GMP_FOUND)
    set(GMP_LIBRARIES "${GMP_LIBRARY}")
    set(GMP_INCLUDE_DIRS "${GMP_INCLUDE_DIR}")
    
    # handle GMPXX
    if(GMPXX_LIBRARY AND GMPXX_INCLUDE_DIR)
        set(GMPXX_FIND_QUIETLY ${GMP_FIND_QUIETLY})
        find_package(PackageMessage)
        find_package_message(GMPXX "Found GMPXX: ${GMPXX_LIBRARY}"
                "[${GMPXX_LIBRARY}][${GMPXX_INCLUDE_DIR}]")
        
        set(GMPXX_FOUND TRUE)
        set(GMPXX_LIBRARIES "${GMP_LIBRARY}" "${GMPXX_LIBRARY}")
        set(GMPXX_INCLUDE_DIRS "${GMP_INCLUDE_DIR}" "${GMPXX_INCLUDE_DIR}")
    endif()
endif(GMP_FOUND)
