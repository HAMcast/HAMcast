# SYNOPSIS
#
#   AX_GMP()
#
# DESCRIPTION
#
#   Test for the Gnu Multiple Precision library
#
# LAST MODIFICATION
#
#   2008-12-11

AC_DEFUN([AX_GMP],
[
  AC_CHECK_HEADERS(gmp.h,,
    [AC_MSG_ERROR([GNU Multiple Precision headers not found (GMP, http://gmplib.org)])])
    
  AC_CHECK_LIB([gmp],[__gmpz_init],,
    [AC_MSG_ERROR([GNU Multiple Precision library not found (GMP, http://gmplib.org)])])
])
