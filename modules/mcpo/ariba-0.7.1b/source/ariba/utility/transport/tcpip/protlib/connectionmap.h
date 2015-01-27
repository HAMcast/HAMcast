/// ----------------------------------------*- mode: C++; -*--
/// @file connectionmap.h
/// maintains connection mapping of application addresses to sockets and vice versa
/// ----------------------------------------------------------
/// $Id: connectionmap.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/connectionmap.h $
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
#ifndef CONNECTION_MAP_H
#define CONNECTION_MAP_H

#include "assocdata.h"
#include <boost/unordered_map.hpp>
using boost::unordered_map;

namespace protlib {



/* @class ConnectionMap 
 * maintains connection mapping of application addresses to sockets and vice versa
 * @ingroup protlib
 * @{
 */
class ConnectionMap {
	public:
		// a constructor may be needed here in this class
		/// Insert a new AssocData element into the ConnectionMap
		bool insert(AssocData* assoc);
		/// Search for existing connections to this specific socket
		AssocData* lookup(socketfd_t socketfd) const;
		/// Search for existing connections to this specific assoc id
		AssocData* lookup(associd_t associd) const;
		///Search for existing connections to this address
		AssocData* lookup(const appladdress& addr) const;
		/// Erase the AssocData-element associated with this socket
		bool erase(socketfd_t socketfd);
		/// Erase the AssocData-element associated with this socket
		bool erase(associd_t associd);
		/// Erase the AssocData-element
		bool erase(AssocData* assoc);
		/// clear all
		void clear();
		/// get number of records
		size_t get_size() const;
	private:
		// this unordered_map uses the standard hashfunction on the first entry, int
		
		// only typedefs
		typedef unordered_map<socketfd_t ,AssocData*> ass2data_t;
		typedef ass2data_t::const_iterator const_ass2data_it_t;
		typedef unordered_map<appladdress,AssocData*> addr2data_t;
		typedef addr2data_t::const_iterator const_addr2data_it_t;

		// internal hashmaps
		ass2data_t ass2data; ///< map: socket fd to association data
		addr2data_t addr2data; ///< map: (application) address to association data
	public:
		/// connection map iterator
		typedef const_ass2data_it_t const_it_t;
		const_it_t begin() const;
		const_it_t end() const;
}; // end class ConnectionMap

inline
size_t 
ConnectionMap::get_size() const { return ass2data.size(); }

inline
ConnectionMap::const_it_t ConnectionMap::begin() const {
	return ass2data.begin();
} // end begin

inline
ConnectionMap::const_it_t ConnectionMap::end() const {
	return ass2data.end();
} // end end

inline
bool 
ConnectionMap::erase(AssocData* assoc) {
  return assoc ? erase(assoc->socketfd) : false;
} // end erase

//@}

} // end namespace protlib
#endif
