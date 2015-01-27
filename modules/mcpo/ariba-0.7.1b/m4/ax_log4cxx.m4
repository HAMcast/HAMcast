# SYNOPSIS
#
#   AX_LOG4CXX()
#
# DESCRIPTION
#
#   Check installation of Apache C++ logging library log4cxx, version
#   0.10.0 or greater.
#   
#
# LAST MODIFICATION
#
#   2010-03-09

AC_DEFUN([AX_LOG4CXX],
[
  AC_LANG_PUSH([C++])

  AC_CHECK_HEADERS([[log4cxx/logger.h]],
  [

    AC_MSG_CHECKING([log4cxx library files (version >=0.10.0)])

    dnl Save old $LIBS to eventually restore it.
    ax_log4cxx_save_libs="$LIBS"
    LIBS="$LIBS -llog4cxx"
  
    AC_LINK_IFELSE([AC_LANG_PROGRAM(
      [[
        #include <log4cxx/logger.h>
        #include <log4cxx/basicconfigurator.h>
      ]], [
      log4cxx::BasicConfigurator::configure();
      log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();
      logger->setLevel(log4cxx::Level::getDebug());
      LOG4CXX_INFO(logger, "Simple message text.");])],
      dnl Success
      AC_MSG_RESULT(yes)
      [LIBS="-llog4cxx $LIBS"]
      AC_DEFINE(HAVE_LIBLOG4CXX, [1], "Define to 1 if you have log4cxx library installed")
      , dnl failed
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([[Could not find log4cxx library.
Version 0.10.0 or greater is required.
(http://logging.apache.org/log4cxx/index.html)]])
      dnl restore old lib state
      LIBS="$ax_log4cxx_save_libs")
    
    AC_LANG_POP
  ]

  ,[AC_MSG_WARN([Could not find log4cxx headers (http://logging.apache.org/log4cxx/index.html), logging will be directly to STDOUT])])

])
