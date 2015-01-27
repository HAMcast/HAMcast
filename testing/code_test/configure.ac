###############################################################################
#                          HAMcast program template                           #
###############################################################################

AC_PREREQ([2.6])

## replace "name_of_your_program" with your (lowercase!) name
## set your current version (default is set to "0.1")
AC_INIT([test], [0.1])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects]) #keine NEWS, README, AUTHORS, CHANGELOG files
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CXX
AC_PROG_RANLIB

## modify if you need a higher boost version than 1.36.1
AX_BOOST_BASE([1.36.1])

## comment out (or delete) all lines you don't need
## important: hamcast requires:
##            - AX_BOOST_ASIO
##            - AX_BOOST_REGEX
##            - AX_BOOST_THREAD
##            - AX_BOOST_DATE_TIME
##            - AX_BOOST_FILESYSTEM
AX_HAMCAST
AX_BOOST_ASIO
AX_BOOST_REGEX
AX_BOOST_THREAD
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM
AX_BOOST_UNIT_TEST_FRAMEWORK

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

