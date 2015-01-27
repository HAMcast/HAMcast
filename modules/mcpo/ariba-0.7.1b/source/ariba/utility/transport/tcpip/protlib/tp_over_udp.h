/// ----------------------------------------*- mode: C++; -*--
/// @file tp_over_udp.h
/// Transport over UDP
/// ----------------------------------------------------------
/// $Id: tp_over_udp.h 2718 2007-07-24 03:23:14Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/include/tp_over_udp.h $
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
/** @ingroup protlib
 * @ file
 * TP over UDP
 */

#ifndef TP_OVER_UDP_H
#define TP_OVER_UDP_H

#include <boost/unordered_map.hpp>

#include "tp.h"
#include "threads.h"
#include "threadsafe_db.h"
#include "connectionmap.h"
#include "assocdata.h"

namespace protlib 
{
/** this struct conatains parameters that determine 
  * the behavior of listener and receiver threads in TPoverUDP
  * @param port - port number for master listener thread (server port)
  * @param sleep - time (in ms) that listener and receiver wait at a poll() call
  * @param d - destination module, where internal message are sent
  */
struct TPoverUDPParam : public ThreadParam 
{
  /// constructor
    TPoverUDPParam(
		 unsigned short common_header_length,
		 bool (*const getmsglength) (NetMsg& m, uint32& clen_words),
		 port_t listen_port,
		 uint32 sleep = ThreadParam::default_sleep_time,
		 bool debug_pdu = false,
		 message::qaddr_t source = message::qaddr_tp_over_udp,
		 message::qaddr_t dest = message::qaddr_signaling,
		 bool sendaborts = false,
		 uint8 tos = 0x10) :
    ThreadParam(sleep,"TPoverUDP",1,1),
    port(listen_port),
    debug_pdu(debug_pdu),
    source(source),
    dest(dest),
    common_header_length(common_header_length),
    getmsglength(getmsglength),
    terminate(false),
    ip_tos(tos)
	{};

    /// port to bind master listener thread to
    const port_t port;
    bool debug_pdu;
    /// message source
    const message::qaddr_t source;
    const message::qaddr_t dest;
    /// what is the length of the common header
    const unsigned short common_header_length;
    
    /// function pointer to a function that figures out the msg length in number of 4 byte words
    /// it returns false if error occured (e.g., malformed header), result is returned in variable clen_words
    bool (*const getmsglength) (NetMsg& m, uint32& clen_words);
    
    /// should master thread terminate?
    const bool terminate;
    const uint8 ip_tos;
    
    bool (*rao_lookup) (uint32);
    
    
}; // end TPoverUDPParam
    

/// TP over UDP
/** This class implements the TP interface using UDP. */
class TPoverUDP : public TP, public Thread 
{
/***** inherited from TP *****/
public:
  /// sends a network message, spawns receiver thread if necessary
  virtual void send(NetMsg* msg, const address& addr, bool use_existing_connection);
  virtual void terminate(const address& addr);
  
  /***** inherited from Thread *****/
public:
  /// main loop
  virtual void main_loop(uint32 nr);
  
/***** other members *****/
public:
  /// constructor
  TPoverUDP(const TPoverUDPParam& p) :
    TP(tsdb::getprotobyname("udp"),"udp",p.name,p.common_header_length,p.getmsglength),
    Thread(p), tpparam(p), already_aborted(false), msgqueue(NULL), debug_pdu(p.debug_pdu),
    master_listener_socket(-1)
  { 
    // perform some initializing actions
    // currently not required (SCTP had to init its library)
    init= true; ///< init done;
  }
  /// virtual destructor
  virtual ~TPoverUDP();

  class sender_thread_start_arg_t
  {
  public:
    TPoverUDP* instance;
    FastQueue* sender_thread_queue;
    
    sender_thread_start_arg_t(TPoverUDP* instance, FastQueue* sq) :
      instance(instance), sender_thread_queue(sq) {};
  };

  int get_listener_socket() const { return master_listener_socket; }
  
private:


  /// send a message to the network via UDP
  void udpsend(NetMsg* msg, appladdress* addr);
  
  /// a static starter method to invoke the listener thread
  static void* listener_thread_starter(void *argp);

  /// listener thread procedure
  void listener_thread();

  /// terminates all active receiver or sender threads
  void terminate_all_threads();
  
  /// parameters for main TPoverUDP thread
  const TPoverUDPParam tpparam;
  
  /// did we already abort at thread shutdown
  bool already_aborted;
  /// message queue
  FastQueue* msgqueue;
  
  bool debug_pdu;

  int master_listener_socket;

}; // end class TPoverUDP

/** A simple internal message for selfmessages
 * please note that carried items may get deleted after use of this message 
 * the message destructor does not delete any item automatically
 */
class TPoverUDPMsg : public message 
{
 public:
  // message type start/stop thread, send data
  enum msg_t { start, 
	       stop,
	       send_data
  };

 private:
  const AssocData* peer_assoc;
  const TPoverUDPMsg::msg_t type;
  NetMsg* netmsg;
  appladdress* addr;

public:
  TPoverUDPMsg(const AssocData* peer_assoc, message::qaddr_t source= qaddr_unknown, TPoverUDPMsg::msg_t type= stop) : 
    message(type_transport, source), peer_assoc(peer_assoc), type(type), netmsg(0), addr(0)  {}

  TPoverUDPMsg(NetMsg* netmsg, appladdress* addr, message::qaddr_t source= qaddr_unknown) : 
    message(type_transport, source), peer_assoc(0), type(send_data), netmsg(netmsg), addr(addr) {}

  const AssocData* get_peer_assoc() const { return peer_assoc; }
  TPoverUDPMsg::msg_t get_msgtype() const { return type; }
  NetMsg* get_netmsg() const { return netmsg; }
  appladdress* get_appladdr() const { return addr; } 
};

} // end namespace protlib

#endif
