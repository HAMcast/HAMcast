// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Universität Karlsruhe (TH)
//
// Institute of Telematics
// Universität Karlsruhe (TH)
// Zirkel 2, 76128 Karlsruhe
// Germany
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE INSTITUTE OF TELEMATICS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ARIBA PROJECT OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation
// are those of the authors and should not be interpreted as representing
// official policies, either expressed or implied, of the Institute of
// Telematics.
// [License]

#ifndef LOGGING_H__
#define LOGGING_H__

#include <iostream>
#include <cstdlib>
#include "ariba/config.h"

#ifdef HAVE_LOG4CXX_LOGGER_H
	#include <log4cxx/logger.h>
	#include <log4cxx/basicconfigurator.h>
#else
	#define ARIBA_LOGGING
#endif // HAVE_LOG4CXX_LOGGER_H

#ifdef LOGCOLORS
  #define colorDefault { std::cout << "\033[0m";    } /*reset*/
  #define colorDebug   { std::cout << "\033[0;33m"; } /*cyan*/
  #define colorInfo    { std::cout << "\033[0;32m"; } /*green*/
  #define colorWarn    { std::cout << "\033[0;34m"; } /*blue*/
  #define colorError   { std::cout << "\033[0;31m"; } /*red*/
#else
  #define colorDefault { }
  #define colorDebug   { }
  #define colorInfo    { }
  #define colorWarn    { }
  #define colorError   { }
#endif // ENABLE_LOGCOLORS


#ifdef HAVE_LOG4CXX_LOGGER_H 

  #define use_logging_h(x) \
  	private: static log4cxx::LoggerPtr logger;

#ifdef ARIBA_LOGGING
  #define use_logging_cpp(x) \
	log4cxx::LoggerPtr x::logger(log4cxx::Logger::getLogger(#x));
#else
  #define use_logging_cpp(x)
#endif

  #define logging_trace(x)  {            LOG4CXX_TRACE(logger,x);                         }
  #define logging_debug(x)  {colorDebug; LOG4CXX_DEBUG(logger,x); colorDefault;           }
  #define logging_info(x)   {colorInfo;  LOG4CXX_INFO(logger,x);  colorDefault;           }
  #define logging_warn(x)   {colorWarn;  LOG4CXX_WARN(logger,x);  colorDefault;           }
  #define logging_error(x)  {colorError; LOG4CXX_ERROR(logger,x); colorDefault;           }
  #define logging_fatal(x)  {colorError; LOG4CXX_FATAL(logger,x); colorDefault; exit(-1); }

  #define logging_rootlevel_debug()	  {log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug()); }
  #define logging_rootlevel_info()	  {log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getInfo() ); }
  #define logging_rootlevel_warn()	  {log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getWarn() ); }
  #define logging_rootlevel_error()	  {log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError()); }

  #define logging_classlevel_debug(x) {log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(#x)); if(logger != NULL) logger->setLevel(log4cxx::Level::getDebug()); }
  #define logging_classlevel_info(x)  {log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(#x)); if(logger != NULL) logger->setLevel(log4cxx::Level::getInfo());  }
  #define logging_classlevel_warn(x)  {log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(#x)); if(logger != NULL) logger->setLevel(log4cxx::Level::getWarn());  }
  #define logging_classlevel_error(x) {log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger(#x)); if(logger != NULL) logger->setLevel(log4cxx::Level::getError()); }

#else // HAVE_LOG4CXX_LOGGER_H

  #define use_logging_h(x)
  #define use_logging_cpp(x)

#ifdef ARIBA_LOGGING
  #define logging_stdout(x) // std::cout << x << std::endl;
#else
  #define logging_stdout(x)
#endif

  static int __loglevel__ = 2; //default is info

  #define logging_trace(x)  {                                   logging_stdout(x);                }
  #define logging_debug(x)  {if(__loglevel__ <= 1){ colorDebug; logging_stdout(x); colorDefault; }}
  #define logging_info(x)   {if(__loglevel__ <= 2){ colorInfo;  logging_stdout(x); colorDefault; }}
  #define logging_warn(x)   {if(__loglevel__ <= 3){ colorWarn;  logging_stdout(x); colorDefault; }}
  #define logging_error(x)  {                       colorError; logging_stdout(x); colorDefault;           }
  #define logging_fatal(x) {                       colorError; logging_stdout(x); colorDefault; exit(-1); }

  #define logging_rootlevel_debug()	  {__loglevel__ = 1;}
  #define logging_rootlevel_info()	  {__loglevel__ = 2;}
  #define logging_rootlevel_warn()	  {__loglevel__ = 3;}
  #define logging_rootlevel_error()	  {__loglevel__ = 4;}

  #define logging_classlevel_debug(x) {std::cout << "individual class logging only available with log4cxx library" << std::endl;}
  #define logging_classlevel_info(x)  {std::cout << "individual class logging only available with log4cxx library" << std::endl;}
  #define logging_classlevel_warn(x)  {std::cout << "individual class logging only available with log4cxx library" << std::endl;}
  #define logging_classlevel_error(x) {std::cout << "individual class logging only available with log4cxx library" << std::endl;}

#endif // HAVE_LOG4CXX_LOGGER_H

#endif //LOGGING_H__
