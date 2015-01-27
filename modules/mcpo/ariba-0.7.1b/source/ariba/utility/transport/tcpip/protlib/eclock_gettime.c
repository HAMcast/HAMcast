/// ----------------------------------------*- mode: C++; -*--
/// @file eclock_gettime.c
/// emulates a clock_gettime call for systems not having it
/// ----------------------------------------------------------
/// $Id: eclock_gettime.c 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/fastqueue/eclock_gettime.c $
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
/** @addtogroup protlib
 * @{
 */



#include <sys/time.h>
#include <unistd.h>

/* struct timezone tz = { 0, DST_NONE }; */
static struct timeval tv;

/* syntax for clock_gettime:
   int clock_gettime (clockid_t clock_id, struct timespec *tp);

   to supply it include the following (because of speed):
   extern int eclock_gettime(struct timespec *tp);
   #define clock_gettime(clock_id, tspec) eclock_gettime(tspec)
*/

int eclock_gettime(struct timespec *tp)
{
  /* DESCRIPTION

  The clock_gettime function returns the current time (in seconds and
  nanoseconds) for the specified clock.  The clock_settime function sets the
  specified clock. The CLOCK_REALTIME clock measures the amount of time
  elapsed since 00:00:00:00 January 1, 1970 Greenwich Mean Time (GMT), other-
  wise known as the Epoch. Time values  that fall between two non-negative
  integer multiples of the resolution are truncated down to the smaller mul-
  tiple of the resolution.

  */
  if (gettimeofday(&tv, 0) == 0)
  {
#ifdef DEBUG
    if (tp)
#endif
    {
      tp->tv_sec= tv.tv_sec;
      tp->tv_nsec= tv.tv_usec*1000;
      return 0;
    }
  }
  else
    return -1;
}

//@}
