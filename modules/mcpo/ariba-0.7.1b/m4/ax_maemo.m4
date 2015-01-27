# SYNOPSIS
#
#   AX_MAEMO()
#
# DESCRIPTION
#
#   Check for Maemo and set specific flags
#
# LAST MODIFICATION
#
#   2009-06-26

AC_DEFUN([AX_MAEMO],
[
	AC_MSG_CHECKING([for maemo platform])
	
	if test "$host" = "arm-unknown-linux-gnu" ; then
		AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_MAEMO, [1], "Define to 1 for the Maemo platform")
	else
		AC_MSG_RESULT(no)
	fi
])
