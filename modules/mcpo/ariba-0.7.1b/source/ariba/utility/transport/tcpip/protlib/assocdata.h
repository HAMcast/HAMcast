/// ----------------------------------------*- mode: C++; -*--
/// @file assocdata.h
/// association data for signaling transport connnections
/// -- AssocData structure to store data of a signaling associaton
/// -- i.e., a socket-based signaling transport connection
/// ----------------------------------------------------------
/// $Id: assocdata.h 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/assocdata.h $
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
#ifndef ASSOC_DATA_H
#define ASSOC_DATA_H

#include "address.h"

namespace protlib {

/**
  * @ingroup protlib
  * @{
  */

typedef int socketfd_t; ///< socket type interface
typedef unsigned int associd_t; /// SCTP lib interface

/// association data, used to keep all necessary information
/// (socket, peer address, shutdown, touched) about a single connection
struct AssocData {
  AssocData(socketfd_t socketfd,
	    const appladdress& peeraddress,
	    const appladdress& ownaddress):
    socketfd(socketfd),
    assoc(0),
    peer(peeraddress),
    ownaddr(ownaddress),
    thread_ID(0),
    num_of_out_streams(0),
    shutdown(false),
    touched(true)
  {};

  AssocData(associd_t ass, const appladdress& ap, const appladdress& oa, uint32 streams)
    : socketfd(0),
      assoc(ass),
      peer(ap),
      ownaddr(oa),
      thread_ID(0),
      num_of_out_streams(streams),
      shutdown(false),
      touched(true)
  {};

  AssocData(associd_t ass, const char* apstr, protocol_t proto, port_t port, uint32 streams, bool& res)
    : socketfd(0),
      assoc(ass),
      peer(apstr,proto,port,&res),
      thread_ID(0),
      num_of_out_streams(streams),
      shutdown(false),
      touched(true)
  {};


  const socketfd_t socketfd; ///< socket of signaling transport connection
  const associd_t assoc; ///< required for SCTP

  const appladdress peer; ///< address of the signaling peer
  const appladdress ownaddr; ///< own endpoint address of the signaling connection

  pthread_t thread_ID; ///< related receiver thread

  const uint32 num_of_out_streams; ///< required for SCTP

  // shutdown: connection is being shutdown, shutdown
  // is not complete yet
  bool shutdown;
  // this is required for a second changce algorithm when cleaning up unused connections
  bool touched;
}; // end AssocData

//@}

} // end namespace protlib
#endif
