/// ----------------------------------------*- mode: C++; -*--
/// @file assocdata_uds.h
/// association data for Unix Domain transport connnections
/// -- AssocData structure to store data of a signaling associaton
/// -- i.e., a socket-based signaling transport connection
//
/// ----------------------------------------------------------
/// $Id: assocdata_uds.h 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/assocdata_uds.h $
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

#ifndef ASSOC_DATA_UDS_H
#define ASSOC_DATA_UDS_H

#include "address.h"


namespace protlib {

/** @ingroup protlib
  * @{
  */

typedef int socketfd_t; ///< socket type interface
typedef unsigned int associd_t; /// SCTP lib interface

/// association data, used to keep all necessary information
/// (socket, peer address, shutdown, touched) about a single connection
struct AssocDataUDS {
  AssocDataUDS(socketfd_t socketfd,
	    const udsaddress& peeraddress,
	    const udsaddress& ownaddress):
    socketfd(socketfd),
    assoc(0),
    peer(peeraddress),
    ownaddr(ownaddress),
    thread_ID(0),
    num_of_out_streams(0),
    shutdown(false),
    touched(true)
  {};

  AssocDataUDS(associd_t ass, const udsaddress& ap, const udsaddress& oa, uint32 streams)
    : socketfd(0),
      assoc(ass),
      peer(ap),
      ownaddr(oa),
      thread_ID(0),
      num_of_out_streams(streams),
      shutdown(false),
      touched(true)
  {};

  const socketfd_t socketfd; ///< socket of signaling transport connection
  const associd_t assoc; ///< required for SCTP

  const udsaddress peer; ///< address of the signaling peer
  const udsaddress ownaddr; ///< own endpoint address of the signaling connection

  pthread_t thread_ID; ///< related receiver thread

  const uint32 num_of_out_streams; ///< required for SCTP

  // shutdown: connection is being shutdown, shutdown
  // is not complete yet
  bool shutdown;
  // this is required for a second changce algorithm when cleaning up unused connections
  bool touched;
}; // end AssocDataUDS

//@}

} // end namespace protlib
#endif
