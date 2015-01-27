AC_INIT([mcpo], [0.5.1], [www.ariba-underlay.org])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([source/mcpo/MCPO.cpp])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([source/mcpo/config.h])

AC_ARG_ENABLE(debug, [  --enable-debug	Enable debug options], enable_debug=$enableval, enable_debug=no)
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)

AC_ARG_ENABLE(profiling, [  --enable-profiling	Enable profiling], enable_profiling=$enableval, enable_profiling=no)
AM_CONDITIONAL(PROFILING, test "$enable_profiling" = yes)

AC_ARG_ENABLE(simulation, [  --enable-simulation	Enable simulation mode], enable_simulation=$enableval, enable_simulation=no)
AM_CONDITIONAL(OMNET, test "$enable_simulation" = yes)

AC_ARG_ENABLE(doxygen, [  --enable-doxygen	Enable doxygen documentation ], enable_doxygen=$enableval, enable_doxygen=no)
AM_CONDITIONAL(DOXYGEN, test "$enable_doxygen" = yes)

AC_ARG_ENABLE(clio, [  --enable-clio	Enable CLIO support [default=yes]], enable_clio=$enableval, enable_clio=yes)

AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_LANG(C++)

dnl Check for doxygen and features
AX_PROG_DOXYGEN

dnl Check for boost
BOOST_REQUIRE([1.35])
BOOST_THREADS
BOOST_SYSTEM
BOOST_FOREACH

dnl Check for logging
AX_LOG4CXX
AX_ARIBA

dnl Check for CLIO
AX_CLIO

AC_CONFIG_FILES([
Makefile
source/Makefile
source/mcpo/Makefile
sample/Makefile
sample/mcpo-pingpong/Makefile
docu/Makefile
docu/doxygen/Makefile
])
AC_OUTPUT
