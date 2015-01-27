/// ----------------------------------------*- mode: C++; -*--
/// @file connectionmap_uds.cpp
/// stores network connection related data
/// ----------------------------------------------------------
/// $Id: connectionmap_uds.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/connectionmap_uds.cpp $
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
#include "connectionmap_uds.h"
#include "logfile.h"

namespace protlib {
  using namespace log;
  using boost::unordered_map;

/** @ingroup protlib
 * @ingroup protlib
 * @{
 */


/***** class ConnectionMapUDS *****/

/** @class ConnectionMapUDS connectionmap.h
    The class ConnectionMapUDS saves all required information about
    currently existing connections. Its methods allow to check for
    connections and return, if possible, the data associated
    with the connection.
*/


/** @returns true if data is not NULL, if there is not already
 * an entry for data and if  
 * it is inserted into the internal hash maps.
 * 
 */
bool ConnectionMapUDS::insert(AssocDataUDS* assoc) 
{
  if (assoc) 
  {
    if ( (assoc->socketfd && lookup(assoc->socketfd)) || 
	 (assoc->assoc && lookup(assoc->assoc)) ||
	 lookup(assoc->peer)) return false;
    else 
    {
      if (assoc->socketfd)
	ass2data[assoc->socketfd] = assoc;
      else
      if (assoc->assoc)
	ass2data[assoc->assoc] = assoc;
      else
	Log(ERROR_LOG,LOG_NORMAL,"ConnectionMapUDS","insertion failed, both socketfd and associd are 0");

      addr2data[assoc->peer] = assoc;
      return true;
    } // end if already in map
  } else return false;
} // end insert

/** @returns a pointer to the AssocDataUDS object or NULL.
 * @param socketfd socket file descriptor
 */

AssocDataUDS* ConnectionMapUDS::lookup(socketfd_t socketfd) const 
{
	const_ass2data_it_t hit= ass2data.find(socketfd);
	if (hit!=ass2data.end()) return hit->second;
	else return NULL;
} // end lookup

AssocDataUDS* ConnectionMapUDS::lookup(associd_t associd) const 
{
	const_ass2data_it_t hit= ass2data.find(associd);
	if (hit!=ass2data.end()) return hit->second;
	else return NULL;
} // end lookup

/** @returns a pointer to the AssocDataUDS object or NULL.
 * @param addr IP-adress + port
 *
 */
AssocDataUDS* ConnectionMapUDS::lookup(const udsaddress& addr) const 
{
	const_addr2data_it_t hit= addr2data.find(addr);
	if (hit!=addr2data.end()) return hit->second;
	else return NULL;
} // end lookup

/** @returns true if the AssocDataUDS-object with socketfd
 * could be deleted
 * @param socketfd socket file descriptor
 */
bool ConnectionMapUDS::erase(socketfd_t socketfd) {
	bool res = true;
	AssocDataUDS* d = lookup(socketfd);
	if (d) {
		if (!ass2data.erase(d->socketfd)) res = false;
		if (!addr2data.erase(d->peer)) res = false;
		delete d; // AssocDataUDS is deleted
		return res;
	} else return false;
} // end erase

/** 
 * erase an association
 * @param associd - association identifier
 * @returns true if the AssocDataUDS-object with associd could be deleted
 */
bool ConnectionMapUDS::erase(associd_t associd) {
	bool res = true;
	AssocDataUDS* d = lookup(associd);
	if (d) {
		if (!ass2data.erase(d->assoc)) res = false;
		if (!addr2data.erase(d->peer)) res = false;
		delete d; // AssocDataUDS is deleted
		return res;
	} else return false;
} // end erase

/*
 *
 */
void ConnectionMapUDS::clear() {
	const_ass2data_it_t hit;
	for (hit=ass2data.begin();hit!=ass2data.end();hit++) {
		if (hit->second) delete hit->second;
	} // end for hit
	ass2data.clear();
	addr2data.clear();
} // end clear


} // end namespace protlib
/// @}
