/// ----------------------------------------*- mode: C++; -*--
/// @file logfile.h
/// Implementation of a logging stream
/// ----------------------------------------------------------
/// $Id: logfile.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/logfile.h $
// ===========================================================
//
// Copyright (C) 2005-2007, all rights reserved by
// - Institute of Telematics, Universitaet Karlsruhe (TH)
//
// More information and contact:
// https://projekte.tm.uka.de/trac/NSIS
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ===========================================================
#ifndef _logfile_h_
#define _logfile_h_


#define _NO_LOGGING

#include <fstream> // file stream
#include <iostream>

#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h> // sprintf
#include <unistd.h> // getpid

#include "protlib_types.h"

namespace protlib {

  using namespace std;
/** @addtogroup protlib
 * @{
 */

namespace log {

static const pthread_mutex_t initlogmutex=
    #ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
		PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    #else
        PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
    #endif

#undef ERROR_LOG
#undef WARNING_LOG
#undef EVENT_LOG
#undef INFO_LOG
#undef DEBUG_LOG
#undef LOG_TYPES

#undef LOG_ALL
#undef LOG_EMERG
#undef LOG_ALERT
#undef LOG_CRIT
#undef LOG_NORMAL
#undef LOG_UNIMP


enum logclass_t
{
  ERROR_LOG= 0x10,
  WARNING_LOG= 0x20,
  EVENT_LOG= 0x30,
  INFO_LOG=  0x40,
  DEBUG_LOG= 0x50,
  EVERY_LOG= 0xF0,
  LOG_TYPES= 5
};


enum loglevel_t
{
  LOG_EMERG=  0,
  LOG_ALERT=  1,
  LOG_CRIT=   2,
  LOG_NORMAL= 4,
  LOG_UNIMP=  8,
  LOG_ALL=   15
};

// colors

enum color_t
{
clear,
bold_on,
italics_on,
underline_on,
inverse_on,
strikethrough_on,
bold_off,
italics_off,
underline_off,
inverse_off,
strikethrough_off,
black,
red,
green,
yellow,
blue,
magenta,
cyan,
white,
fg_default,
bg_black,
bg_red,
bg_green,
bg_yellow,
bg_blue,
bg_magenta,
bg_cyan,
bg_white,
bg_default,
num_colors,
off=0};

extern const char* color[];
extern const char *const ANSIcolorcode[];


#define  ERRLog(mname, logstring) Log(ERROR_LOG, LOG_NORMAL, mname, logstring)
#define  ERRCLog(mname, logstring) Log(ERROR_LOG, LOG_CRIT, mname, logstring)
#define  EVLog(mname, logstring) Log(EVENT_LOG, LOG_NORMAL, mname, logstring)
#define  WLog(mname, logstring) Log(WARNING_LOG, LOG_NORMAL, mname, logstring)
#define  ILog(mname, logstring) Log(INFO_LOG, LOG_NORMAL, mname, logstring)
#define  DLog(mname, logstring) Log(DEBUG_LOG, LOG_NORMAL, mname, logstring)


#ifndef _NO_LOGGING

// Log(lclass, llevel, mname, logstring)
// lclass: logclass, llevel: severitylevel, mname: module or methodname,
// logstring: things to log in stream notation
#define  Log(lclass, llevel, mname, logstring) do {			\
  if ( protlib::log::DefaultLog.should_log(lclass, llevel) ) {		\
    using protlib::log::DefaultLog;					\
    DefaultLog.logstart(lclass, llevel, mname) << logstring;		\
    DefaultLog.logend();						\
  }									\
} while ( false )

#define  LogS(lclass, llevel, mname, logstring) do {			\
  if ( protlib::log::DefaultLog.should_log(lclass, llevel) ) {		\
    protlib::log::DefaultLog.logstart(lclass, llevel, mname,		\
	__FILE__, __FUNCTION__, __LINE__) << logstring;			\
    protlib::log::DefaultLog.logend();					\
  }									\
} while ( false )

#else

#define  Log(logclass, loglevel, mname, logstring)
#define  LogS(logclass, loglevel, mname, logstring)

#endif

class logfile
{
 private:
  ostream* logstream;
  pthread_mutex_t logmutex;

  unsigned char logfilter[LOG_TYPES];

  bool usecolors;
  bool quiet_start;

  const char* timenow();

 public:

  logfile(const char* filename="", bool usecolors= true, bool quietstart=false);
  ~logfile();

  bool set_dest(const char* filename, bool quiet=false);
  void set_filter(logclass_t logclass, uint8 severitylevel);
  bool should_log(logclass_t logclass, loglevel_t severitylevel);


  ostream& logstart(logclass_t logclass, loglevel_t severity_level,
		    const string& modname,
		    const char* file="",
		    const char* func="",
		    int line=0);
  void coloron() { usecolors=true; for (int i= 0; i<num_colors; i++) {color[i]= ANSIcolorcode[i]; } }
  void coloroff() { usecolors=false; for (int i= 0; i<num_colors; i++) {color[i]= ""; } }

  void logend();
}; // end class logfile

extern

inline
logfile::logfile(const char* filename,  bool usecolors, bool quietstart)
  : logstream(0),
    logmutex(initlogmutex),
    usecolors(usecolors),
    quiet_start(quietstart)
{
  for (int i= 0; i< LOG_TYPES; i++)
    logfilter[i]= LOG_ALL;

  if (strlen(filename))
    logstream= new(nothrow) ofstream(filename);
  else
    logstream= &cout;

  if (!logstream)
  {
    cerr << "Could not open logfile " << filename << endl;
  }
  else
  {
    pthread_mutex_lock(&logmutex);

    // if enable colors, use ANSI code,
    // if disable colors, so replace all strings with "";
    for (int i= 0; i<num_colors; i++)
    {
      color[i]= (usecolors ? ANSIcolorcode[i] : "");
    }

    if (!quiet_start && logstream)
    {
//       (*logstream) << color[blue] << timenow()
// 		   << '[' << getpid() << "] >>>>>>>>>>>>>>>>>>>>>>>> *** LOG START *** >>>>>>>>>>>>>>>>>>>>>>>>"
// 		   << color[off] << endl;
    }
    pthread_mutex_unlock(&logmutex);
  }
}


inline
logfile::~logfile()
{
  if (logstream)
  {
    pthread_mutex_lock(&logmutex);

    if ( ! quiet_start )
// 	(*logstream) << color[blue] << timenow() << '[' << getpid()
// 	<< "] <<<<<<<<<<<<<<<<<<<<<<<< *** LOG  STOP *** <<<<<<<<<<<<<<<<<<<<<<<<"
// 	<< color[off] << endl;
    pthread_mutex_unlock(&logmutex);

    // destroy mutex
    pthread_mutex_destroy(&logmutex);

    // delete if allocated stream
    if (logstream!= &cout)
      delete logstream;

    logstream= 0;
  }
}



inline
void
logfile::set_filter(logclass_t logclass, uint8 severity_level)
{
  uint8 logclass_index= (logclass>>4)-1;
  if (logclass_index < LOG_TYPES)
    logfilter[logclass_index]=  severity_level;
}

inline
bool
logfile::should_log(logclass_t logclass, loglevel_t severity_level)
{
  return severity_level <= logfilter[(logclass>>4)-1];
}


/**
 * returns current time in static char array
 * @return pointer to static character array that contains current time
 */
inline
const char* logfile::timenow()
{
  static time_t t;
  static struct timeval now;
  static char timestr[32]= "";
  static char msecstr[6];

  gettimeofday(&now,NULL);
  t= now.tv_sec;
  strftime(timestr, sizeof(timestr)-sizeof(msecstr), "%Y-%m-%d %H:%M:%S.", localtime(&t));
  snprintf(msecstr,sizeof(msecstr),"%03lu",now.tv_usec/1000UL);
  strcat(timestr,msecstr);
  return timestr;
}

inline
void logfile::logend()
{
  if (logstream)
  {
    (*logstream) << color[off] << endl;
  }

  pthread_mutex_unlock(&logmutex);
}


extern logfile& DefaultLog;
//@}

} // end namespace log

} // end namespace protlib





/**
 * returns current time in static char array
 * @return pointer to static character array that contains current time
 */
inline
const char* log_timenow(char* module, char* event)
{

    //static pthread_mutex_t timestampmutex= PTHREAD_MUTEX_INITIALIZER;



    //pthread_mutex_lock(&timestampmutex);


    static time_t t;
    //static struct timeval now;
    static char timestr[128]= "";
    //static char msecstr[8];

    struct timespec tp;

    clock_gettime(CLOCK_REALTIME, &tp);

    //gettimeofday(&now,NULL);
    t= tp.tv_sec;

    //bringing down the range to under 1 hour
    //write out microseconds

    //printf("%ld\n", tp.tv_nsec);
    //printf("%ld\n", t % 3600);

    sprintf(timestr, "%05ld%06ld - ", t % 3600, tp.tv_nsec/1000L);
    //snprintf(msecstr,sizeof(msecstr),"%06lu",tp.tv_nsec/1000000UL);
    strcat(timestr,module);
    strcat(timestr," - ");
    strcat(timestr,event);

    //pthread_mutex_unlock(&timestampmutex);

    return timestr;
}



#endif
