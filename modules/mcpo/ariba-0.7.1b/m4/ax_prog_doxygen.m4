# SYNOPSIS
#
#   AX_PROG_DOXYGEN()
#
# DESCRIPTION
#
#   Test for the Doxygen
#
#   --enable-doxygen option enable or disable generation of the
#   documentation, when available.
#
# LAST MODIFICATION
#
#   2008-12-11

AC_DEFUN([AX_PROG_DOXYGEN],
[
AC_ARG_VAR([DOXYGEN_PROG],[Doxygen program])

AC_ARG_ENABLE([doxygen],
  AS_HELP_STRING([--enable-doxygen], [Turn on doxygen documentation generation.]))

AM_CONDITIONAL([DOXYGEN], [test "x${enable_doxygen}" = "xyes"])

if test "${enable_doxygen}" = "yes"; then
   dnl Check for existance of doxygen
   AC_CHECK_PROG([DOXYGEN_PROG],
     [doxygen],
     doxygen,
     no)
fi

if test "$DOXYGEN_PROG" = "no"; then
   AC_MSG_ERROR([Doxygen was not found. Check for installation or configure with --disable-doxygen])
fi
])
