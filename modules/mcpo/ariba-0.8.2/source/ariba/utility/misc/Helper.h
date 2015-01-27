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

#ifndef __HELPER_H
#define __HELPER_H

#include <string>
#include <list>
#include <cassert>
#include <ctime>
#include <ostream>
#include <cstdio>
#include <iomanip>
#include <cassert>
#include <ostream>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

using std::list;
using std::string;
using std::setfill;
using std::setw;
using std::string;
using std::ostream;
using std::ostringstream;

namespace ariba {
namespace utility {

namespace Helper {

	//
	// string conversion functions
	//

	/// unsigned long to string
	string ultos(unsigned long val);

	/// pointer to string-address
	template<class T>
	string ptos(T pnt) {
		std::ostringstream oss;
		oss << "0x" << std::hex << pnt;
		return oss.str();
	}

	/// long to string
	string ltos(long val);

	/// unsigned long to hex string
	string ultohexs(unsigned long val, bool hexdelimiter = true);

	/// long to hex string
	string ltohexs(long val, bool hexdelimiter = true);

	/// string to long
	long stol(string str);

	/// string to int
	int stoi(string str);

	/// string to double
	double stod(string str);

	/// hex string to unsigned int
	unsigned int hstoui(string str);

	/// double to string
	string dtos(double val);

	//
	// string manipulation functions
	//

	/// trim string
	string trim(string str);

	/// split a string into substrings. The single strings trimmed from whitespace
	/// only strings that have a resulting length > 0 after the trim are in the list
	typedef list<string> STRING_LIST;
	typedef STRING_LIST::iterator STRING_LIST_ITERATOR;
	STRING_LIST split(string str, string delimiter);

	/// replace all occurences of find in the string str with repl
	string replace(string str, string find, string repl);

	//
	// time functions
	//

	string getTime(time_t timeval = 0);
	string getDate(time_t timeval = 0);
	unsigned long getElapsedMillis();
	void sleep(unsigned int millis);

	//
	// constants
	//

	#ifdef WIN32
	const string LINE_BREAK = "\r\n";
	#else
	const string LINE_BREAK = "\n";
	#endif

}; // namespace Helper

inline void Helper::sleep(unsigned int millis) {
#ifdef WIN32
	Sleep (millis);
#else

	unsigned long secondsSleep = millis / 1000;
	unsigned long millisSleep = millis % 1000;

	//
	// sleep the seconds part of the time
	// (usleep can not sleep more than 999999 microseconds
	//  which is a little bit too less for a second)
	//

	if (secondsSleep > 0)
		::sleep(secondsSleep);

	//
	// sleep the microsecond part of the time
	//

	if (millisSleep > 0) {
		assert (millisSleep < 1000);
		usleep(millisSleep * 1000);
	}
#endif
}

}} // namespace ariba, common

#endif // __HELPER_H
