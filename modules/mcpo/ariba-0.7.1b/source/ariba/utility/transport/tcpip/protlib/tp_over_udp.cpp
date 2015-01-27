/// ----------------------------------------*- mode: C++; -*--
/// @file tp_over_udp.cpp
/// UDP-based transport module
/// ----------------------------------------------------------
/// $Id: tp_over_udp.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/tp_over_udp.cpp $
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

extern "C"
{
  //#define _SOCKADDR_LEN    /* use BSD 4.4 sockets */
#include <unistd.h>		/* gethostname */
#include <sys/types.h>		/* network socket interface */
#include <netinet/ip.h>         /* iphdr */
#include <netinet/ip6.h>        /* ip6_hdr */
#include <netinet/in.h>		/* network socket interface */
#include <netinet/tcp.h>	/* for TCP Socket Option */
#include <netinet/udp.h>	/* for UDP header */
#include <sys/socket.h>
#include <arpa/inet.h>		/* inet_addr */

#ifndef IPPROTO_IPV6
	// v6 structs not defined in netinet/in.h, pull in here
	#include <linux/ipv6.h>
#endif

#include <fcntl.h>
#include <sys/poll.h>
}

#include <iostream>
#include <errno.h>
#include <string>
#include <sstream>

#include "tp_over_udp.h"
#include "threadsafe_db.h"
#include "cleanuphandler.h"
#include "setuid.h"
#include "logfile.h"
#include "linux/netfilter.h"

#include <set>

#define UDP_SUCCESS 0
#define UDP_SEND_FAILURE 1

#define BUFSIZE 2048000


const unsigned int max_listen_queue_size = 10;

namespace protlib
{

  using namespace log;

/** @defgroup protlib
 * @ingroup protlib
 * @{
 */

char in6_addrstr_loc[INET6_ADDRSTRLEN+1];

/******* class TPoverUDP *******/


/** generates an internal TPoverUDP message to send a NetMsg to the network
 *
 *  - the sending thread will call TPoverUDP::udpsend()
 *  - since UDP is connectionless we can safely ignore the use_existing_connection attribute
 *  @note the netmsg is deleted by the send() method when it is not used anymore
 */
void TPoverUDP::send (NetMsg * netmsg, const address & in_addr, bool use_existing_connection)
{

  appladdress* addr = NULL;
  addr= dynamic_cast<appladdress*>(in_addr.copy());

  if (!addr) return;

  // Do it independently from master thread
  udpsend(netmsg, addr);

}

/** sends a NetMsg to the network.
 *
 * @param netmsg   message to send
 * @param addr     transport endpoint address
 *
 * @note           both parameters are deleted after the message was sent
 */
void
TPoverUDP::udpsend (NetMsg * netmsg, appladdress * addr)
{
#ifndef _NO_LOGGING
  const char *const thisproc = "sender   - ";
#endif

  // set result initially to success, set it to failure
  // in places where these failures occur
  int result = UDP_SUCCESS;
  int ret = 0;


  if (addr)
    check_send_args (*netmsg, *addr);
  else
    {
      ERRCLog (tpparam.name, thisproc << "address pointer is NULL");
      result = UDP_SEND_FAILURE;
      throw TPErrorInternal();
    }


  addr->convert_to_ipv6();
  in6_addr ip6addr;

  //convert to v4-mapped address if necessary! (we use dual-stack IPv4/IPv6 socket)
  addr->get_ip(ip6addr);


  // *********************************** revised socket code *********************************


  // msghdr for sendmsg
  struct msghdr header;

  // pointer for ancillary data
  struct cmsghdr *ancillary = NULL;

  // iovec for sendmsg
  struct iovec iov;
  iov.iov_base = netmsg->get_buffer();
  iov.iov_len = netmsg->get_size();

  // destination address
  struct sockaddr_in6 dest_address;
  dest_address.sin6_family= AF_INET6;
  dest_address.sin6_port  = htons(addr->get_port());
  dest_address.sin6_addr  = ip6addr;
  dest_address.sin6_flowinfo = 0;
  dest_address.sin6_scope_id = 0;

  // fill msghdr
  header.msg_iov = &iov;
  header.msg_iovlen = 1;
  header.msg_name = &dest_address;
  header.msg_namelen=sizeof(dest_address);
  header.msg_control=NULL;
  header.msg_controllen=0;


  // pktinfo
  in6_pktinfo pktinfo;

  //addr->set_if_index(1);


  // we have to add up to 2 ancillary data objects (for interface and hop limit)

  uint32 buflength = 0;
  if (addr->get_if_index()) {
    buflength = CMSG_SPACE(sizeof(pktinfo));
    //cout << "PKTINFO data object, total buffer size: " << buflength << "byte" << endl;
  }

  int hlim = addr->get_ip_ttl();

  if (hlim) {
    buflength = buflength + CMSG_SPACE(sizeof(int));
    //cout << "HOPLIMIT data object, total buffer size: " << buflength << "byte" << endl;
  }
  // create the buffer
  if ((addr->get_if_index()) || hlim) {
    header.msg_control = malloc(buflength);
    if (header.msg_control == 0)
      ERRCLog(tpparam.name, thisproc << " malloc failed for ancillary data of size " << buflength);
  }

  // are we to set the outgoing interface?
  if (addr->get_if_index()) {

    DLog(tpparam.name, thisproc << " UDP send via Interface " << addr->get_if_index() << " requested.");

    // first cmsghdr at beginning of buffer
    ancillary = (cmsghdr*) header.msg_control;

    ancillary->cmsg_level=IPPROTO_IPV6;
    ancillary->cmsg_type=IPV6_PKTINFO;
    ancillary->cmsg_len=CMSG_LEN(sizeof(pktinfo));

    //cout << "Set up properties of ancillary data object 1" << endl;

    pktinfo.ipi6_addr = in6addr_any;
    pktinfo.ipi6_ifindex = addr->get_if_index();

    memcpy (CMSG_DATA(ancillary), &pktinfo, sizeof(pktinfo));

    //cout << "Set up data of ancillary data object 1" << endl;

    // update msghdr controllen
    header.msg_controllen = CMSG_SPACE(sizeof(pktinfo));

  }

  // should we set an explicit Hop Limit?
  if (hlim) {
    DLog(tpparam.name, thisproc << " UDP send with IP TTL of " << hlim << " requested.");

    // second cmsghdr after first one
    cmsghdr* ancillary2 = NULL;

    if (ancillary) {
      ancillary2 = (cmsghdr*) (ancillary + CMSG_SPACE(sizeof(pktinfo)));
    } else {
      ancillary2 = (cmsghdr*) header.msg_control;
    }

    ancillary2->cmsg_level=IPPROTO_IPV6;
    ancillary2->cmsg_type=IPV6_HOPLIMIT;
    ancillary2->cmsg_len = CMSG_LEN(sizeof(int));

    memcpy(CMSG_DATA(ancillary2), &hlim, sizeof(int));

    // update msghdr controllen
    header.msg_controllen = header.msg_controllen + ancillary2->cmsg_len;

  }

#ifndef _NO_LOGGING
  uint32 msgsize = netmsg->get_size();	// only used for logging below
#endif

  // check whether socket is already up and initialized by listener thread
  // otherwise we may have a race condition, i.e., trying to send before socket is created
  // FIXME: it may be the case that the socket is already created, but not bound
  //        I'm not sure what happens, when we try to send...
  while (master_listener_socket == -1)
  {
    const unsigned int sleeptime= 1;
    DLog(tpparam.name, "socket not yet ready for sending - sending deferred (" << sleeptime << " s)");
    sleep(sleeptime);
    DLog(tpparam.name, "retrying to send");
  }
  // reset IP RAO option
  ret = setsockopt(master_listener_socket, SOL_IP, IP_OPTIONS, 0, 0);
  if ( ret != 0 )
    ERRLog(tpparam.name, "unsetting IP options for IPv4 failed");

  // send UDP packet
  DLog(tpparam.name, "SEND to " << *addr);
  ret= sendmsg(master_listener_socket,&header,MSG_DONTWAIT);

  if (ret<0)
    ERRCLog(tpparam.name, "Socket Send failed! - error (" << errno << "):" << strerror(errno));
  if (debug_pdu)
    {
      ostringstream hexdump;
      netmsg->hexdump (hexdump);
      Log (DEBUG_LOG, LOG_NORMAL, tpparam.name,
	   "PDU debugging enabled - Sent:" << hexdump.str ());
    }

  if (ret < 0)
    {
      result = UDP_SEND_FAILURE;
      //    break;
    } // end if (ret < 0)


      // *** note: netmsg is deleted here ***
  delete netmsg;


  // Throwing an exception within a critical section does not
  // unlock the mutex.

  if (result != UDP_SUCCESS)
    {
      ERRLog(tpparam.name, thisproc << "UDP error, returns " << ret << ", error : " << strerror (errno));
      delete addr;

      throw TPErrorSendFailed();

    }
  else
    Log (EVENT_LOG, LOG_NORMAL, tpparam.name,
	 thisproc << ">>----Sent---->> message (" << msgsize <<
	 " bytes) using socket " << master_listener_socket << " to " << *addr);

  // *** delete address ***
  delete addr;
} // end TPoverUDP::udpsend



/**
 * IPv4 catcher thread starter:
 * just a static starter method to allow starting the
 * actual master_listener_thread() method.
 *
 * @param argp - pointer to the current TPoverUDP object instance
 */
void *
TPoverUDP::listener_thread_starter (void *argp)
{
  // invoke listener thread method
  if (argp != 0)
    {
	(static_cast < TPoverUDP * >(argp))->listener_thread ();
    }
  return 0;
}





/**
 * UDP master receiver thread: waits for incoming connections at the well-known udp port
 *
 */
void TPoverUDP::listener_thread ()
{
  // create a new address-structure for the listening masterthread
  struct sockaddr_in6 own_address;
  own_address.sin6_family = AF_INET6;
  own_address.sin6_flowinfo= 0;
  own_address.sin6_port = htons(tpparam.port); // use port number in param structure
  // accept incoming connections on all interfaces
  own_address.sin6_addr = in6addr_any;
  own_address.sin6_scope_id= 0;

  // create a listening socket
  master_listener_socket= socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  if (master_listener_socket == -1)
    {
      ERRCLog(tpparam.name, "Could not create a new socket, error: " << strerror(errno));
      return;
    }

  int socketreuseflag= 1;
  int status= setsockopt(master_listener_socket,
			 SOL_SOCKET,
			 SO_REUSEADDR,
			 (const char *) &socketreuseflag,
			 sizeof(socketreuseflag));
  if (status)
  {
    ERRCLog(tpparam.name, "Could not set socket option SO_REUSEADDR:" << strerror(errno));
  }

  // TODO: insert multicast socket options/calls here

  // bind the newly created socket to a specific address
  int bind_status = bind(master_listener_socket,
			 reinterpret_cast<struct sockaddr *>(&own_address),
			 sizeof(own_address));
  if (bind_status)
    {
      ERRCLog(tpparam.name, "Binding to "
	  << inet_ntop(AF_INET6, &own_address.sin6_addr, in6_addrstr_loc, INET6_ADDRSTRLEN)
	  << " port " << tpparam.port << " failed, error: " << strerror(errno));
      return;
    }


  // create a pollfd struct for use in the mainloop
  struct pollfd poll_fd;
  poll_fd.fd = master_listener_socket;
  poll_fd.events = POLLIN | POLLPRI;
  poll_fd.revents = 0;
  /*
    #define POLLIN	0x001	// There is data to read.
    #define POLLPRI	0x002	// There is urgent data to read.
    #define POLLOUT	0x004	// Writing now will not block.
  */

  bool terminate = false;
  // check for thread terminate condition using get_state()
  state_t currstate= get_state();
  int poll_status= 0;
  const unsigned int number_poll_sockets= 1;
  struct sockaddr_in6 peer_address;
  socklen_t peer_address_len;
  // int conn_socket;

  // check whether this thread is signaled for termination
  while(! (terminate= (currstate==STATE_ABORT || currstate==STATE_STOP) ) )
    {


      // wait on number_poll_sockets (main drm socket)
      // for the events specified above for sleep_time (in ms) tpparam.sleep_time
      poll_status= poll(&poll_fd, number_poll_sockets, 250);
      if (poll_fd.revents & POLLERR) // Error condition
	{
	  if (errno != EINTR)
	    {
	      ERRCLog(tpparam.name, "Poll caused error " << strerror(errno) << " - indicated by revents");
	    }
	  else
	    {
	      EVLog(tpparam.name, "poll(): " << strerror(errno));
	    }

	}
      if (poll_fd.revents & POLLHUP) // Hung up
	{
	  ERRCLog(tpparam.name, "Poll hung up");
	  return;
	}
      if (poll_fd.revents & POLLNVAL) // Invalid request: fd not open
	{
	  ERRCLog(tpparam.name, "Poll Invalid request: fd not open");
	  return;
	}

      switch (poll_status)
	{
	case -1:
	  if (errno != EINTR)
	    {
	      ERRCLog(tpparam.name, "Poll status indicates error: " << strerror(errno));
	    }
	  else
	    {
	      EVLog(tpparam.name, "Poll status: " << strerror(errno));
	    }

	  break;

	case 0:
#ifdef DEBUG_HARD
	  Log(DEBUG_LOG,LOG_UNIMP, tpparam.name,
	      "Listen Thread - Poll timed out after " << tpparam.sleep_time << " ms.");
#endif
	  currstate= get_state();
	  continue;
	  break;

	default:
#ifdef DEBUG_HARD
	  Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, "Poll: " << poll_status << " event(s) ocurred, of type " << poll_fd.revents);
#endif
	  break;
	} // end switch



      //if there is data to read, do it

      if ((poll_fd.revents & POLLIN) || (poll_fd.revents & POLLPRI)) {


	// in peer_address and the size of its address in addrlen
	peer_address_len= sizeof(peer_address);

	//Build us a NetMsg
	NetMsg *netmsg=0;
	netmsg = new NetMsg (NetMsg::max_size);



	/// receive data from socket buffer (recv will not block)
	int ret = recvfrom (master_listener_socket,
			    netmsg->get_buffer (), NetMsg::max_size, 0, reinterpret_cast<struct sockaddr *>(&peer_address),
			    &peer_address_len);

	if (ret)
	{
	  DLog(tpparam.name, "Yankeedoo, we received " << ret << " bytes of DATA!!");

	  // truncate netmsg buffer
	  netmsg->truncate(ret);
	}

	/**************************************************************
	 *  The following restrictions should apply:                  *
	 *                                                            *
	 *  This is UDP, messages are contained in ONE datagram       *
	 *  datagrams CANNOT fragment, as otherwise TCP is used       *
	 *  so we now build a TPMsg, send it to signaling and         *
	 *  all should be well. At least until now.                   *
	 **************************************************************/

	// Build peer_adr and own_addr
	appladdress* peer_addr = new appladdress;
	peer_addr->set_ip(peer_address.sin6_addr);
	peer_addr->set_port(peer_address.sin6_port);
	appladdress* own_addr = new appladdress();

	// Log the sender peer and write to peer_addr
	char source_addr[INET6_ADDRSTRLEN+1];
	inet_ntop(AF_INET6, &peer_address.sin6_addr, source_addr, INET6_ADDRSTRLEN);


	peer_addr->set_port(htons(peer_address.sin6_port));
	peer_addr->set_ip(peer_address.sin6_addr);
	peer_addr->set_protocol(get_underlying_protocol());

	DLog(tpparam.name, "Peer: [" << *peer_addr << "]");

	// create TPMsg and send it to the signaling thread
	//fprintf (stderr, "Before TPMsg creation\n");
	TPMsg *tpmsg=
	  new (nothrow) TPMsg (netmsg, peer_addr, own_addr);

	Log (DEBUG_LOG, LOG_NORMAL, tpparam.name,
	     "recvthread - receipt of GIST PDU now complete, sending msg#" << tpmsg->get_id() << " to signaling module");


	if (tpmsg == NULL || !tpmsg->send(tpparam.source, tpparam.dest))
	  {
	    ERRLog(tpparam.name, "rcvthread" << "Cannot allocate/send TPMsg");
	    if (tpmsg)
	      delete tpmsg;
	    if (netmsg)
	      delete netmsg;

	  }

      }

      // get new thread state
      currstate= get_state();

    } // end while(!terminate)

  return;

}


TPoverUDP::~TPoverUDP ()
{
  init = false;

  Log (DEBUG_LOG, LOG_NORMAL, tpparam.name, "Destructor called");

}

/** TPoverUDP Thread main loop.
 * This loop checks for internal messages of either
 * a send() call to start a new receiver thread, or,
 * of a receiver_thread() that signals its own termination
 * for proper cleanup of control structures.
 *
 * @param nr number of current thread instance
 */
void
TPoverUDP::main_loop (uint32 nr)
{

  int pthread_status = 0;


  // start UDP listener thread
  pthread_t listener_thread_ID;
  pthread_status = pthread_create (&listener_thread_ID, NULL,	//NULL: default attributes
				   listener_thread_starter, this);
  if (pthread_status)
    {
      ERRCLog(tpparam.name,
	   "UDP listening thread could not be created: " <<
	   strerror (pthread_status));
    }
  else

    Log(INFO_LOG,LOG_NORMAL, tpparam.name, color[green] << "Listening at port #" << tpparam.port << color[off]);



  // define max latency for thread reaction on termination/stop signal
  state_t currstate = get_state ();

  // check whether this thread is signaled for termination
  while (currstate != STATE_ABORT && currstate != STATE_STOP)
    {

      // get thread state
      currstate = get_state ();

      sleep(4);

    }				// end while

  if (currstate == STATE_STOP)
    {
      // start abort actions
      Log (INFO_LOG, LOG_NORMAL, tpparam.name,
	   "Asked to abort, stopping all receiver threads");
    }				// end if stopped

  // do not accept any more messages
  // terminate all receiver and sender threads that are still active
  //terminate_all_threads ();
}


void
TPoverUDP::terminate(const address& addr)
{
	// no connection oriented protocol, nothing to terminate
}

}				// end namespace protlib

///@}
