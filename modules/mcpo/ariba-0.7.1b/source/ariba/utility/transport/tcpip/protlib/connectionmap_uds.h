/// ----------------------------------------*- mode: C++; -*--
/// @file connectionmap_uds.h
/// maintains connection mapping of application addresses to 
/// UNIX domain sockets and vice versa
/// ----------------------------------------------------------
/// $Id: connectionmap_uds.h 2549 2007-04-02 22:17:37Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/connectionmap_uds.h $
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
#ifndef CONNECTION_MAP_UDS_H
#define CONNECTION_MAP_UDS_H

#include "assocdata_uds.h"
#include <boost/unordered_map.hpp>

namespace protlib {

using boost::unordered_map;

/* @class ConnectionMap 
 * maintains connection mapping of application addresses to sockets and vice versa
 * @ingroup protlib
 * @{
 */
class ConnectionMapUDS {
	public:
		// a constructor may be needed here in this class
		/// Insert a new AssocDataUDS element into the ConnectionMapUDS
		bool insert(AssocDataUDS* assoc);
		/// Search for existing connections to this specific socket
		AssocDataUDS* lookup(socketfd_t socketfd) const;
		/// Search for existing connections to this specific assoc id
		AssocDataUDS* lookup(associd_t associd) const;
		///Search for existing connections to this address
		AssocDataUDS* lookup(const udsaddress& addr) const;
		/// Erase the AssocDataUDS-element associated with this socket
		bool erase(socketfd_t socketfd);
		/// Erase the AssocDataUDS-element associated with this socket
		bool erase(associd_t associd);
		/// Erase the AssocDataUDS-element
		bool erase(AssocDataUDS* assoc);
		/// clear all
		void clear();
		/// get number of records
		size_t get_size() const;
	private:
		// this unordered_map uses the standard hashfunction on the first entry, int
		
		// only typedefs
		typedef unordered_map<socketfd_t ,AssocDataUDS*> ass2data_t;
		typedef ass2data_t::const_iterator const_ass2data_it_t;
		typedef unordered_map<udsaddress,AssocDataUDS*> addr2data_t;
		typedef addr2data_t::const_iterator const_addr2data_it_t;

		// internal hashmaps
		ass2data_t ass2data; ///< map: socket fd to association data
		addr2data_t addr2data; ///< map: (application) address to association data
	public:
		/// connection map iterator
		typedef const_ass2data_it_t const_it_t;
		const_it_t begin() const;
		const_it_t end() const;
}; // end class ConnectionMapUDS

inline
size_t 
ConnectionMapUDS::get_size() const { return ass2data.size(); }

inline
ConnectionMapUDS::const_it_t ConnectionMapUDS::begin() const {
	return ass2data.begin();
} // end begin

inline
ConnectionMapUDS::const_it_t ConnectionMapUDS::end() const {
	return ass2data.end();
} // end end

inline
bool 
ConnectionMapUDS::erase(AssocDataUDS* assoc) {
  return assoc ? erase(assoc->socketfd) : false;
} // end erase

//@}

} // end namespace protlib
#endif
