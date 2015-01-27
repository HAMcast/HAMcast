AC_INIT([ariba], [0.7.1], [http://www.ariba-underlay.org])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([source/ariba/ariba.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADER([source/ariba/config.h])

AC_ARG_ENABLE(debug, [  --enable-debug	Enable debug options], enable_debug=$enableval, enable_debug=no)
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)

AC_ARG_ENABLE(profiling, [  --enable-profiling	Enable profiling], enable_profiling=$enableval, enable_profiling=no)
AM_CONDITIONAL(PROFILING, test "$enable_profiling" = yes)

AC_ARG_ENABLE(simulation, [  --enable-simulation	Enable simulation mode], enable_simulation=$enableval, enable_simulation=no)
AM_CONDITIONAL(OMNET, test "$enable_simulation" = yes)

AC_ARG_ENABLE(doxygen, [  --enable-doxygen	Enable doxygen documentation ], enable_doxygen=$enableval, enable_doxygen=no)
AM_CONDITIONAL(DOXYGEN, test "$enable_doxygen" = yes)

AC_ARG_ENABLE(logcolors, [  --enable-logcolors	Enable colorful logging], enable_logcolors=$enableval, enable_logcolors=no)
AM_CONDITIONAL(LOGCOLORS, test "$enable_logcolors" = yes)

AC_ARG_ENABLE(bluetooth, [  --enable-bluetooth	Enable bluetooth transport [default=yes]], enable_bluetooth=$enableval, enable_bluetooth=yes)

AC_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

AC_LANG(C++)

dnl Check for doxygen and features
AX_PROG_DOXYGEN

dnl Check for libraries
AX_GMP
AX_LOG4CXX
AX_OPENSSL
AX_AVAHI

if test "$enable_bluetooth" = "yes"; then
  AX_BLUETOOTH
fi

dnl Check for boost
BOOST_REQUIRE([1.35])
BOOST_THREADS
BOOST_SYSTEM
BOOST_FOREACH
BOOST_REGEX

dnl Check for maemo platform
AX_MAEMO

AC_CONFIG_FILES([
Makefile
source/Makefile
source/ariba/Makefile
sample/Makefile
sample/pingpong/Makefile
sample/testdht/Makefile
docu/Makefile
docu/doxygen/Makefile
])

AC_OUTPUT
