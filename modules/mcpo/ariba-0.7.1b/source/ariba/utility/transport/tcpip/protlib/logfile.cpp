/// ----------------------------------------*- mode: C++; -*--
/// @file logfile.cpp
/// Implementation of a logging stream
/// ----------------------------------------------------------
/// $Id: logfile.cpp 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/logfile.cpp $
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
#include "logfile.h"
#include <iostream>
#include <iomanip>
#include <new>

#include <time.h>
#include <sys/time.h> // gettimeofday

namespace protlib {

using namespace std;

  namespace log {

const char *const ANSIcolorcode[]= {
  "[0m",  // clear, off; reset; clears all colors and styles (to white on black)
  "[1m",  // bold_on		
  "[3m",  // italics_on	
  "[4m",  // underline_on	
  "[7m",  // inverse_on	
  "[9m",  // strikethrough_on	
  "[22m", // bold_off		
  "[23m", // italics_off	
  "[24m", // underline_off	
  "[27m", // inverse_off	
  "[29m", // strikethrough_off
  "[30m", // black  		
  "[31m", // red    		
  "[32m", // green  		
  "[33m", // yellow 		
  "[34m", // blue   		
  "[35m", // magenta		
  "[36m", // cyan   		
  "[37m", // white  		
  "[39m", // default		
  "[40m", // bg_black  	
  "[41m", // bg_red    	
  "[42m", // bg_green  	
  "[43m", // bg_yellow 	
  "[44m", // bg_blue   	
  "[45m", // bg_magenta	
  "[46m", // bg_cyan   	
  "[47m", // bg_white  	
  "[49m"  // bg_default       
};

const char* color[num_colors+1];

const char *const logclass_str[]=
{
  " ZERO  ",
  "*ERROR*",
  "WARNING",
  " EVENT ",
  " INFO  ",
  " DEBUG ",
  " EXPERT"
};


const char *const logseveritylevel_str[]=
  {
    "EMERG ",
    "ALERT ",
    "CRITIC",
    "LEVEL3",
    "NORMAL",
    "LEVEL5",
    "LEVEL6",
    "LEVEL7",
    "UNIMP ",
    "LEVEL9",
    "LEVELA",
    "LEVELB",
    "LEVELC",
    "LEVELD",
    "LEVELE",
    "-ALL- "
  };


/** set logging destination to new filename
 * @return true if logfile could be opened for given name
 */
bool
logfile::set_dest(const char* filename, bool quiet)
{
  // lock everything
  pthread_mutex_lock(&logmutex); 

  if (logstream && !quiet)
  {
//     (*logstream) << color[blue] << timenow() << '[' << getpid() << "] Redirecting Log output to \"" << filename << '\"' << endl;
//     (*logstream) << color[blue] << timenow() 
// 		 << '[' << getpid() << "] <<<<<<<<<<<<<<<<<<<<<<<< *** LOG  STOP *** <<<<<<<<<<<<<<<<<<<<<<<<" 
// 		 << color[off] << endl;
  }

  // delete old stream
  if (logstream!= &cout)
    delete logstream;

  // allocate new stream
  if (strlen(filename))
    logstream= new(nothrow) ofstream(filename);
  else
    logstream= &cout;

  pthread_mutex_unlock(&logmutex); 

  if (!logstream)
  {
    cerr << "Could not open logfile " << filename << endl;
    return false;
  }
  else
  {
//     (*logstream) << color[blue] << timenow() 
// 		 << '[' << getpid() << "] >>>>>>>>>>>>>>>>>>>>>>>> *** LOG START *** >>>>>>>>>>>>>>>>>>>>>>>>" 
// 		 << color[off] << endl;
  }
  return true;
}


ostream&
logfile::logstart(logclass_t logclass, loglevel_t severity_level, 
		  const string& modname,
		  const char* file, 
		  const char* func, 
		  int line)
{  
  // lock logstream for writing, must be unlocked by logfile.end()
  int mtxlck_ret= 0;
  if ( (mtxlck_ret= pthread_mutex_lock(&logmutex)) )
  {
    cerr << color[red] << "logfile::logstart() ERROR while locking mutex - return code:" << mtxlck_ret << "/" << strerror(mtxlck_ret) << endl;
  }

  if ( logstream )
  {
    (*logstream) << (logclass==ERROR_LOG ? color[bold_on] : "")
                 << (logclass==WARNING_LOG ? color[magenta] : 
		     (logclass==ERROR_LOG ? color[red] : color[off])
		    )
                 << timenow()
		 << '-' << getpid() << (logclass!=ERROR_LOG ? '-' : '*')
		 << color[bold_on] << logclass_str[logclass>>4] << (logclass==ERROR_LOG ? color[bold_on] : color[bold_off])
		 << (logclass!=ERROR_LOG ? '/' : '*') << hex << severity_level << dec
		 << ": " << color[bold_on] << left << setfill(' ') << setw(15) << modname << color[bold_off] << right << " " << color[off];
  }
  
  return (*logstream);
}


  } // end namespace log

} // end namespace protlib
