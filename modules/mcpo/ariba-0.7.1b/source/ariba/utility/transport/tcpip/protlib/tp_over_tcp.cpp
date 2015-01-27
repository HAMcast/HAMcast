/// ----------------------------------------*- mode: C++; -*--
/// @file tp_over_tcp.cpp
/// TCP-based transport module (includes framing support)
/// ----------------------------------------------------------
/// $Id: tp_over_tcp.cpp 2872 2008-02-18 10:58:03Z bless $
/// $HeadURL: https://svn.ipv6.tm.uka.de/nsis/protlib/trunk/src/tp_over_tcp.cpp $
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
#include <unistd.h>      /* gethostname */
#include <sys/types.h>   /* network socket interface */
#include <netinet/in.h>  /* network socket interface */
#include <netinet/tcp.h> /* for TCP Socket Option */
#include <sys/socket.h>
#include <arpa/inet.h>   /* inet_addr */

#include <fcntl.h>
#include <sys/poll.h>
}

#include <iostream>
#include <errno.h>
#include <string>
#include <sstream>

#include "tp_over_tcp.h"
#include "threadsafe_db.h"
#include "cleanuphandler.h"
#include "setuid.h"
#include "queuemanager.h"
#include "logfile.h"

#include <set>

#define TCP_SUCCESS 0
#define TCP_SEND_FAILURE 1

const unsigned int max_listen_queue_size= 10;

#define IPV6_ADDR_INT32_SMP 0x0000ffff

namespace protlib {

void v6_to_v4(struct sockaddr_in *sin, struct sockaddr_in6 *sin6)	{
	bzero(sin, sizeof(*sin));
 	sin->sin_family = AF_INET;
 	sin->sin_port = sin6->sin6_port;
 	memcpy(&sin->sin_addr, &sin6->sin6_addr.s6_addr[12], sizeof(struct in_addr));
}

/* Convert sockaddr_in to sockaddr_in6 in v4 mapped addr format. */
void v4_to_v6(struct sockaddr_in *sin, struct sockaddr_in6 *sin6) {
	bzero(sin6, sizeof(*sin6));
	sin6->sin6_family = AF_INET6;
	sin6->sin6_port = sin->sin_port;
 	*(uint32_t *)&sin6->sin6_addr.s6_addr[0] = 0;
	*(uint32_t *)&sin6->sin6_addr.s6_addr[4] = 0;
 	*(uint32_t *)&sin6->sin6_addr.s6_addr[8] = IPV6_ADDR_INT32_SMP;
	*(uint32_t *)&sin6->sin6_addr.s6_addr[12] = sin->sin_addr.s_addr;
}

using namespace log;

/** @defgroup protlib
 * @ingroup protlib
 * @{
 */

char in6_addrstr[INET6_ADDRSTRLEN+1];


/******* class TPoverTCP *******/


/** get_connection_to() checks for already existing connections. 
 *  If a connection exists, it returns "AssocData" 
 *  and saves it in "connmap" for further use
 *  If no connection exists, a new connection to "addr"
 *  is created.
 */
AssocData* 
TPoverTCP::get_connection_to(const appladdress& addr)
{
  // get timeout
  struct timespec ts;
  get_time_of_day(ts);
  ts.tv_nsec+= tpparam.sleep_time * 1000000L;
  if (ts.tv_nsec>=1000000000L)
  {
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec= ts.tv_nsec % 1000000000L;
  }

  // create a new AssocData pointer, initialize it to NULL
  AssocData* assoc= NULL;
  int new_socket;
  // loop until timeout is exceeded: TODO: check applicability of loop
  do 
  {
    // check for existing connections to addr
    // start critical section
    lock(); // install_cleanup_thread_lock(TPoverTCP, this);
    assoc= connmap.lookup(addr);
    // end critical section
    unlock(); // uninstall_cleanup(1);
    if (assoc) 
    {
      // If not shut down then use it, else abort, wait and check again.
      if (!assoc->shutdown) 
      {
	return assoc;
      }
      else
      {
	// TODO: connection is already in shutdown mode. What now?
	ERRCLog(tpparam.name,"socket exists, but is already in mode shutdown");

	return 0;
      }  
    } //end __if (assoc)__
    else 
    {
      Log(DEBUG_LOG,LOG_UNIMP,tpparam.name,"No existing connection to " 
	  << addr.get_ip_str() << " port #" << addr.get_port() << " found, creating a new one.");
    }

    // no connection found, create a new one
    new_socket = socket( v4_mode ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (new_socket == -1)
    {
      ERRCLog(tpparam.name,"Couldn't create a new socket: " << strerror(errno));

      return 0;
    }

    // Disable Nagle Algorithm, set (TCP_NODELAY)
    int nodelayflag= 1;
    int status= setsockopt(new_socket,
			   IPPROTO_TCP,
			   TCP_NODELAY,
			   &nodelayflag,
			   sizeof(nodelayflag));
    if (status)
    {
        ERRLog(tpparam.name, "Could not set socket option TCP_NODELAY:" << strerror(errno));
    }
    
    // Reuse ports, even if they are busy
    int socketreuseflag= 1;
    status= setsockopt(new_socket,
			   SOL_SOCKET,
			   SO_REUSEADDR,
			   (const char *) &socketreuseflag,
			   sizeof(socketreuseflag));
    if (status)
    {
        ERRLog(tpparam.name, "Could not set socket option SO_REUSEADDR:" << strerror(errno));
    }

	struct sockaddr_in6 dest_address;
	dest_address.sin6_flowinfo= 0;
	dest_address.sin6_scope_id= 0;
	addr.get_sockaddr(dest_address);

	// connect the socket to the destination address
	int connect_status = 0;
	if (v4_mode) {
		struct sockaddr_in dest_address_v4;
		v6_to_v4( &dest_address_v4, &dest_address );
		connect_status = connect(new_socket,
					 reinterpret_cast<const struct sockaddr*>(&dest_address_v4),
					 sizeof(dest_address));
	} else {
		connect_status = connect(new_socket,
					 reinterpret_cast<const struct sockaddr*>(&dest_address),
					 sizeof(dest_address));
	}
      
    // connects to the listening_port of the peer's masterthread    
    if (connect_status != 0)
    {
      ERRLog(tpparam.name,"Connect to " << addr.get_ip_str() << " port #" << addr.get_port() 
	  << " failed: [" << color[red] << strerror(errno) << color[off] << "]");

      close(new_socket);
      return 0; // error: couldn't connect
    }


    struct sockaddr_in6 own_address;
	if (v4_mode) {
		struct sockaddr_in own_address_v4;
		socklen_t own_address_len_v4 = sizeof(own_address_v4);
		getsockname(new_socket, reinterpret_cast<struct sockaddr*>(&own_address_v4), &own_address_len_v4);
		v4_to_v6(&own_address_v4, &own_address);
	} else {
		socklen_t own_address_len= sizeof(own_address);
		getsockname(new_socket, reinterpret_cast<struct sockaddr*>(&own_address), &own_address_len);
	}

    Log(DEBUG_LOG,LOG_UNIMP, tpparam.name,">>--Connect-->> to " << addr.get_ip_str() << " port #" << addr.get_port() 
	<< " from " << inet_ntop(AF_INET6,&own_address.sin6_addr,in6_addrstr,INET6_ADDRSTRLEN) 
	<< " port #" << ntohs(own_address.sin6_port));

   
    // create new AssocData record (will copy addr)
    assoc = new(nothrow) AssocData(new_socket, addr, appladdress(own_address,IPPROTO_TCP));

    // if assoc could be successfully created, insert it into ConnectionMap
    if (assoc) 
    {
      bool insert_success= false;
      // start critical section
      lock(); // install_cleanup_thread_lock(TPoverTCP, this);
      // insert AssocData into connection map
      insert_success= connmap.insert(assoc);
      // end critical section
      unlock(); // uninstall_cleanup(1);

      if (insert_success == true) 
      {
#ifdef _DEBUG
	Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, "Connected to " << addr.get_ip_str() << ", port #" << addr.get_port()
	    << " via socket " << new_socket);

		
#endif

	// notify master thread to start a receiver thread (i.e. send selfmessage)
	TPoverTCPMsg* newmsg= new(nothrow)TPoverTCPMsg(assoc, tpparam.source, TPoverTCPMsg::start);
	if (newmsg)
	{
	  bool ret = newmsg->send_to(tpparam.source);
	  if(!ret) delete newmsg;
	  return assoc;
	}
	else
	  ERRCLog(tpparam.name,"get_connection_to: could not get memory for internal msg");
      } 
      else 
      {
	// delete data and abort
	ERRCLog(tpparam.name, "Cannot insert AssocData for socket " << new_socket << ", "<< addr.get_ip_str()
	    <<", port #" << addr.get_port() << " into connection map, aborting connection");
		
	// abort connection, delete its AssocData
	close (new_socket);
	if (assoc) 
	{ 
	  delete assoc; 
	  assoc= 0;
	}
	return assoc;
      } // end else connmap.insert			
      
    } // end "if (assoc)"
  } 
  while (wait_cond(ts)!=ETIMEDOUT);

  return assoc;
} //end get_connection_to


/** terminates a signaling association/connection
 *  - if no connection exists, generate a warning
 *  - otherwise: generate internal msg to related receiver thread
 */
void
TPoverTCP::terminate(const address& in_addr)
{
#ifndef _NO_LOGGING
 const char *const thisproc="terminate()   - ";
#endif

 appladdress* addr = NULL;
 addr = dynamic_cast<appladdress*>(in_addr.copy());

 if (!addr) return;

  // Create a new AssocData-pointer 
  AssocData* assoc = NULL;

  // check for existing connections to addr

  // start critical section:
  // please note if receiver_thread terminated already, the assoc data is not
  // available anymore therefore we need a lock around cleanup_receiver_thread()

  lock(); // install_cleanup_thread_lock(TPoverTCP, this);
  assoc= connmap.lookup(*addr);
  if (assoc) 
  {
    EVLog(tpparam.name,thisproc<<"got request to shutdown connection for peer " << addr);
    // If not shut down then use it, else abort, wait and check again.
    if (!assoc->shutdown) 
    {
      if (assoc->socketfd)
      {
	// disallow sending
	if (shutdown(assoc->socketfd,SHUT_WR))
	{
	  ERRLog(tpparam.name,thisproc<<"shutdown (write) on socket for peer " << addr << " returned error:" << strerror(errno));
	}
	else
        {
	  EVLog(tpparam.name,thisproc<<"initiated closing of connection for peer " << addr << ". Shutdown (write) on socket "<< assoc->socketfd );	  
	}
      }
      assoc->shutdown= true;
    }
    else
      EVLog(tpparam.name,thisproc<<"connection for peer " << addr << " is already in mode shutdown");
      
  }
  else
    WLog(tpparam.name,thisproc<<"could not find a connection for peer " << *addr);

  stop_receiver_thread(assoc);

  // end critical section
  unlock(); // uninstall_cleanup(1);

  if (addr) delete addr;
}


/** generates and internal TPoverTCP message to send a NetMsg to the network
 *  - it is necessary to let a thread do this, because the caller
 *     may get blocked if the connect() or send() call hangs for a while
 *  - the sending thread will call TPoverTCP::tcpsend()
 *  - if no connection exists, creates a new one (unless use_existing_connection is true)
 *  @note the netmsg is deleted by the send() method when it is not used anymore
 */
void
TPoverTCP::send(NetMsg* netmsg, const address& in_addr, bool use_existing_connection)
{
  if (netmsg == NULL) {
    ERRCLog(tpparam.name,"send() - called without valid NetMsg buffer (NULL)");
    return;
  }

  appladdress* addr = NULL;
  addr= dynamic_cast<appladdress*>(in_addr.copy());
    
  if (!addr)
  {
    ERRCLog(tpparam.name,"send() - given destination address is not of expected type (appladdress), has type " << (int) in_addr.get_type());
    return;
  } 

  // lock due to sendermap access
  lock();
    
  // check for existing sender thread
  sender_thread_queuemap_t::const_iterator it= senderthread_queuemap.find(*addr);
    
  FastQueue* destqueue= 0;

  if (it == senderthread_queuemap.end())
  { // no sender thread found so far

    // if we already have an existing connection it is save to create a sender thread
    // since get_connection_to() will not be invoked, so an existing connection will
    // be used
    const AssocData* assoc = connmap.lookup(*addr);

    //DLog(tpparam.name,"send() - use_existing_connection:" << (use_existing_connection ? "true" : "false") << " assoc:" << assoc);

    if (use_existing_connection==false || (assoc && assoc->shutdown==false && assoc->socketfd>0))
    { // it is allowed to create a new connection for this thread
      // create a new queue for sender thread
      FastQueue* sender_thread_queue= new FastQueue;
      create_new_sender_thread(sender_thread_queue);
      // remember queue for later use
      
      //pair<sender_thread_queuemap_t::iterator, bool> tmpinsiterator=
      senderthread_queuemap.insert( pair<appladdress,FastQueue*> (*addr,sender_thread_queue) );
      
      destqueue= sender_thread_queue;
    }
  }
  else
  { // we have a sender thread, use stored queue from map
    destqueue= it->second; 
  }
    
  unlock();
    
  // send a send_data message to it (if we have a destination queue)
  if (destqueue)
  {
    // both parameters will be freed after message was sent!
    
    appladdress* apl=new appladdress(*addr);
    TPoverTCPMsg* internalmsg= new TPoverTCPMsg(netmsg,apl);
    if (internalmsg)
    {
      // send the internal message to the sender thread queue
      bool sent = internalmsg->send(tpparam.source,destqueue);
      if (!sent) {
    	  delete internalmsg->get_appladdr();
    	  delete internalmsg;
    	  internalmsg = NULL;
      }
    }
  }
  else
  {
    if (!use_existing_connection)
      WLog(tpparam.name,"send() - found entry for address, but no active sender thread available for peer addr:" << *addr << " - dropping data");
    else
    {
      DLog(tpparam.name,"no active sender thread found for peer " << *addr << " - but policy forbids to set up a new connection, will drop data");

    }
    // cannot send data, so we must drop it
    delete netmsg;
  }
  
  if (addr) delete addr;
}

/** sends a NetMsg to the network.
 *
 * @param netmsg message to send
 * @param addr   transport endpoint address
 *
 * @note   if no connection exists, creates a new one 
 * @note   both parameters are deleted after the message was sent
 */
void
TPoverTCP::tcpsend(NetMsg* netmsg, appladdress* addr)
{                        
#ifndef _NO_LOGGING
  const char *const thisproc="sender   - ";
#endif

  // set result initially to success, set it to failure
  // in places where these failures occur
  int result = TCP_SUCCESS;
  int saved_errno= 0;
  int ret= 0;

  // Create a new AssocData-pointer 
  AssocData* assoc = NULL;
  
  // tp.cpp checks for initialisation of tp and correctness of
  // msgsize, protocol and ip,
  // throws an error if something is not right
  if (addr) {
    addr->convert_to_ipv6();
    check_send_args(*netmsg,*addr);
  } 
  else 
  {
    ERRCLog(tpparam.name, thisproc << "address pointer is NULL");
    result= TCP_SEND_FAILURE;

    delete netmsg;
    delete addr;

    throw TPErrorInternal();
  }


  // check for existing connections, 
  // if a connection exists, return its AssocData 
  // and save it in assoc for further use
  // if no connection exists, create a new one (in get_connection_to()).
  // Connection is inserted into connection map that must be done
  // with exclusive access
  assoc= get_connection_to(*addr);

  if (assoc==NULL || assoc->socketfd<=0)
  {
    ERRCLog(tpparam.name, color[red] << thisproc << "no valid assoc/socket data - dropping packet");

    delete netmsg;
    delete addr;
    return;
  }

  if (assoc->shutdown)
  {
    Log(WARNING_LOG, LOG_ALERT, tpparam.name, thisproc << "should send message although connection already half closed");
    delete netmsg;
    delete addr;

    throw TPErrorSendFailed();
  }

  uint32 msgsize= netmsg->get_size();
#ifdef DEBUG_HARD
  cerr << thisproc << "message size=" << netmsg->get_size() << endl;
#endif

  const uint32 retry_send_max = 3;
  uint32 retry_count = 0;
  // send all the data contained in netmsg to the socket
  // which belongs to the address "addr"
  for (uint32 bytes_sent= 0;
       bytes_sent < msgsize;
       bytes_sent+= ret)
  {

#ifdef _DEBUG_HARD
    for (uint32 i=0;i<msgsize;i++)
    {
      cout << "send_buf: " << i << " : "; 
      if ( isalnum(*(netmsg->get_buffer()+i)) )
	cout << "'" << *(netmsg->get_buffer()+i) << "' (0x" << hex << (unsigned short) *(netmsg->get_buffer()+i) << dec << ")" ;
      else
	cout << "0x" << hex << (unsigned short) *(netmsg->get_buffer()+i) << dec;
      cout << endl;
    }

    cout << endl;
    cout << "bytes_sent: " << bytes_sent << endl;
    cout << "Message size: " <<  msgsize << endl;
    cout << "Send-Socket: " << assoc->socketfd << endl;
    cout << "pointer-Offset. " << netmsg->get_pos() << endl;
    cout << "vor send " << endl;
#endif

    retry_count= 0;
    do 
    {
      // socket send
      ret= ::send(assoc->socketfd, 
		  netmsg->get_buffer() + bytes_sent,  
		  msgsize - bytes_sent, 
		  MSG_NOSIGNAL);
      
      // send_buf + bytes_sent

      // Deal with temporary internal errors like EAGAIN, resource temporary unavailable etc.
      // retry sending
      if (ret < 0)
      { // internal error during send occured
	saved_errno= errno;
	switch(saved_errno)
	{
	  case EAGAIN:  // same as EWOULDBLOCK
          case EINTR:   // man page says: A signal occurred before any data was transmitted.
	  case ENOBUFS: //  The output queue for a network interface was full.
	    retry_count++;
	    ERRLog(tpparam.name,"Temporary failure while calling send(): " << strerror(saved_errno) << ", errno: " << saved_errno 
		   << " - retry sending, retry #" << retry_count);
	    // pace down a little bit, sleep for a while
	    sleep(1);
	    break;

	    // everything else should not lead to repetition 
	  default:
	    retry_count= retry_send_max;
	    break;
	} // end switch
      } // endif
      else // leave while
	break;
    } 
    while(retry_count < retry_send_max);

    if (ret < 0) 
    { // unrecoverable error occured
      
      result= TCP_SEND_FAILURE;
      break;
    } // end if (ret < 0)
    else
    {
      if (debug_pdu)
      {
	ostringstream hexdump;
	netmsg->hexdump(hexdump,netmsg->get_buffer(),bytes_sent);
	DLog(tpparam.name,"PDU debugging enabled - Sent:" << hexdump.str());
      }
    }

    // continue with send next data block in next for() iteration
  } // end for

  // *** note: netmsg is deleted here ***
  delete netmsg;
  
  // Throwing an exception within a critical section does not 
  // unlock the mutex.

  if (result != TCP_SUCCESS)
  {
    ERRLog(tpparam.name, thisproc << "TCP error, returns " << ret << ", error : " << strerror(errno));
    delete addr;

    throw TPErrorSendFailed(saved_errno);
    
  }
  else
    EVLog(tpparam.name, thisproc << ">>----Sent---->> message (" << msgsize << " bytes) using socket " << assoc->socketfd  << " to " << *addr);

  if (!assoc) {
    // no connection
    
    ERRLog(tpparam.name, thisproc << "cannot get connection to " << addr->get_ip_str() 
	<< ", port #" << addr->get_port());

    delete addr;

    throw TPErrorUnreachable(); // should be no assoc found
  } // end "if (!assoc)"
  
  // *** delete address ***
  delete addr;
}
  
/* this thread waits for an internal message that either:
 * - requests transmission of a packet
 * - requests to stop this thread
 * @param argp points to the thread queue for internal messages
 */
void 
TPoverTCP::sender_thread(void *argp)
{
#ifndef _NO_LOGGING
  const char *const methodname="senderthread - ";
#endif

  message* internal_thread_msg = NULL;

  EVLog(tpparam.name, methodname << "starting as thread <" << pthread_self() << ">");

  FastQueue* fq= reinterpret_cast<FastQueue*>(argp);
  if (!fq)
  {
    ERRLog(tpparam.name, methodname << "thread <" << pthread_self() << "> no valid pointer to msg queue. Stop.");
    return;
  }

  bool terminate= false;
  TPoverTCPMsg* internalmsg= 0;
  while (terminate==false && (internal_thread_msg= fq->dequeue()) != 0 )
  {
    internalmsg= dynamic_cast<TPoverTCPMsg*>(internal_thread_msg);
    
    if (internalmsg == 0)
    {
      ERRLog(tpparam.name, methodname << "received not an TPoverTCPMsg but a" << internal_thread_msg->get_type_name());
    }
    else
    if (internalmsg->get_msgtype() == TPoverTCPMsg::send_data)
    {
      // create a connection if none exists and send the netmsg
      if (internalmsg->get_netmsg() && internalmsg->get_appladdr())
      {
	try 
	{
	  tcpsend(internalmsg->get_netmsg(),internalmsg->get_appladdr());
	} // end try
	catch(TPErrorSendFailed& err)
	{
	  ERRLog(tpparam.name, methodname << "TCP send call failed - " << err.what() 
		 << " cause: (" << err.get_reason() << ") " << strerror(err.get_reason()) );
	} // end catch
	catch(TPError& err)
	{
	  ERRLog(tpparam.name, methodname << "TCP send call failed - reason: " << err.what());
	} // end catch
	catch(...)
	{
	  ERRLog(tpparam.name, methodname << "TCP send call failed - unknown exception");
	}
      }
      else
      {
	ERRLog(tpparam.name, methodname << "problem with passed arguments references, they point to 0");
      } 
    }
    else
    if (internalmsg->get_msgtype() == TPoverTCPMsg::stop)
    {
      terminate= true;
    }

    delete internalmsg;
  } // end while
  
  EVLog(tpparam.name, methodname << "<" << pthread_self() << "> terminated connection.");
}


/** receiver thread listens at a TCP socket for incoming PDUs 
 *  and passes complete PDUs to the coordinator. Incomplete
 *  PDUs due to aborted connections or buffer overflows are discarded.
 *  @param argp - assoc data and flags sig_terminate and terminated
 *  
 *  @note this is a static member function, so you cannot use class variables
 */
void 
TPoverTCP::receiver_thread(void *argp)
{
#ifndef _NO_LOGGING
  const char *const methodname="receiver - ";
#endif

  receiver_thread_arg_t *receiver_thread_argp= static_cast<receiver_thread_arg_t *>(argp);
  const appladdress* peer_addr = NULL;
  const appladdress* own_addr = NULL;
  uint32 bytes_received = 0;
  TPMsg* tpmsg= NULL;
  
  // argument parsing - start
  if (receiver_thread_argp == 0)
  {
    ERRCLog(tpparam.name, methodname << "No arguments given at start of receiver thread <" << pthread_self() << ">, exiting.");

    return;
  }
  else
  {
    // change status to running, i.e., not terminated
    receiver_thread_argp->terminated= false;

#ifdef _DEBUG
    DLog(tpparam.name, methodname << "New receiver thread <" << pthread_self() << "> started. ");
#endif
  }

  int conn_socket= 0;
  if (receiver_thread_argp->peer_assoc)
  {
    // get socket descriptor from arg
    conn_socket = receiver_thread_argp->peer_assoc->socketfd;
    // get pointer to peer address of socket (source/sender address of peer) from arg
    peer_addr= &receiver_thread_argp->peer_assoc->peer;
    own_addr= &receiver_thread_argp->peer_assoc->ownaddr;
  }
  else
  {
    ERRCLog(tpparam.name, methodname << "No peer assoc available - pointer is NULL");
    
    return;
  }

  if (peer_addr == 0)
  {
    ERRCLog(tpparam.name, methodname << "No peer address available for socket " << conn_socket << ", exiting.");
    
    return;
  }
  // argument parsing - end
#ifdef _DEBUG
  Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, methodname <<
      "Preparing to wait for data at socket " 
      << conn_socket << " from " << receiver_thread_argp->peer_assoc->peer);
#endif

  int ret= 0;
  uint32 msgcontentlength= 0;
  bool msgcontentlength_known= false;
  bool pdu_complete= false; // when to terminate inner loop

  /* maybe use this to create a new pdu, 
    /// constructor
    contextlistpdu(type_t t, subtype_t st, uint32 fc, uint32 numobj = 0);
  */ 

  // activate O_NON_BLOCK  for recv on socket conn_socket
  // CAVEAT: this also implies non-blocking send()!
  //fcntl(conn_socket,F_SETFL, O_NONBLOCK);
    
  // set options for polling
  const unsigned int number_poll_sockets= 1; 
  struct pollfd poll_fd;
  // have to set structure before poll call
  poll_fd.fd = conn_socket;
  poll_fd.events = POLLIN | POLLPRI; 
  poll_fd.revents = 0;

  int poll_status;
  bool recv_error= false;

  NetMsg* netmsg= 0;
  NetMsg* remainbuf= 0;
  size_t buffer_bytes_left= 0;
  size_t trailingbytes= 0;
  bool skiprecv= false;
  // loop until we receive a terminate signal (read-only var for this thread) 
  // or get an error from socket read
  while( receiver_thread_argp->sig_terminate == false )
  {
    // Read next PDU from socket or process trailing bytes in remainbuf
    ret= 0;
    msgcontentlength= 0;
    msgcontentlength_known= false;
    pdu_complete= false;
    netmsg= 0;

    // there are trailing bytes left from the last receive call
    if (remainbuf != 0)
    {
      netmsg= remainbuf;
      remainbuf= 0;
      buffer_bytes_left= netmsg->get_size()-trailingbytes;
      bytes_received= trailingbytes;
      trailingbytes= 0;
      skiprecv= true;
    }
    else // no trailing bytes, create a new buffer
    if ( (netmsg= new NetMsg(NetMsg::max_size)) != 0 )
    {
      buffer_bytes_left= netmsg->get_size();
      bytes_received= 0;
      skiprecv= false;
    }
    else
    { // buffer allocation failed
      bytes_received= 0;
      buffer_bytes_left= 0;
      recv_error= true;
    }
    
    // loops until PDU is complete
    // >>>>>>>>>>>>>>>>>>>>>>>>>>> while >>>>>>>>>>>>>>>>>>>>>>>>
    while (!pdu_complete && 
	   !recv_error && 
	   !receiver_thread_argp->sig_terminate)
    {
      if (!skiprecv)
      {
	// read from TCP socket or return after sleep_time
	poll_status= poll(&poll_fd, number_poll_sockets, tpparam.sleep_time);
	
	if (receiver_thread_argp->sig_terminate)
	{
	  Log(EVENT_LOG,LOG_UNIMP,tpparam.name,methodname << "Thread <" << pthread_self() << "> found terminate signal after poll");
	  // disallow sending
	  AssocData* myassoc=const_cast<AssocData *>(receiver_thread_argp->peer_assoc);
	  if (myassoc->shutdown == false)
	  {
	    myassoc->shutdown= true;
	    if (shutdown(myassoc->socketfd,SHUT_WR))
	    {
	      if ( errno != ENOTCONN )
		Log(ERROR_LOG,LOG_UNIMP,tpparam.name,methodname <<"shutdown (write) on socket " << conn_socket << " returned error:" << strerror(errno));
	    }
	  }
	  // try to read do a last read from the TCP socket or return after sleep_time
	  if (poll_status == 0)
	  {
	    poll_status= poll(&poll_fd, number_poll_sockets, tpparam.sleep_time);
	  }
	}

	if (poll_fd.revents & POLLERR) // Error condition
	{
	  if (errno == 0 || errno == EINTR)
	  {
	    EVLog(tpparam.name, methodname << "poll(): " << strerror(errno));
	  }
	  else
	  {
	    ERRCLog(tpparam.name, methodname << "Poll indicates error: " << strerror(errno));
	    recv_error= true;
	  }
	}
	
	if (poll_fd.revents & POLLHUP) // Hung up 
	{
	  Log(EVENT_LOG,LOG_CRIT, tpparam.name, methodname << "Poll hung up");
	  recv_error= true;
	}
	
	if (poll_fd.revents & POLLNVAL) // Invalid request: fd not open
	{
	  EVLog(tpparam.name, methodname << "Poll Invalid request: fd not open");
	  recv_error= true;
	}
	
	// check status (return value) of poll call
	switch (poll_status)
	{
	  case -1:
	    if (errno == 0 || errno == EINTR)
	    {
	      EVLog(tpparam.name, methodname << "Poll status: " << strerror(errno));
	    }
	    else
	    {
	      ERRCLog(tpparam.name, methodname << "Poll status indicates error: " << strerror(errno) << "- aborting");
	      recv_error= true;
	    }
	    
	    continue; // next while iteration
	    break;
	    
	  case 0:
#ifdef DEBUG_HARD
	    Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, methodname << "Poll timed out after " << tpparam.sleep_time << " ms.");
#endif
	    continue; // next while iteration
	    break;
	    
	  default:
#ifdef DEBUG_HARD
	    Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, methodname << "Poll: " << poll_status << " event(s) ocurred, of type " << poll_fd.revents);
#endif
	    break;
	} // end switch


	/// receive data from socket buffer (recv will not block)
	ret = recv(conn_socket, 
		   netmsg->get_buffer() + bytes_received, 
		   buffer_bytes_left, 
		   MSG_DONTWAIT);

	if ( ret < 0 )
	{
	  delete netmsg;
	  if (errno!=EAGAIN && errno!=EWOULDBLOCK)
	  {
	    ERRCLog(tpparam.name, methodname << "Receive at socket " << conn_socket << " failed, error: " << strerror(errno));
	    recv_error= true;
	    continue;
	  }
	  else
	  { // errno==EAGAIN || errno==EWOULDBLOCK
	    // just nothing to read from socket, continue w/ next poll
	    continue;
	  }
	}
	else
	{
	  if (ret == 0)
	  {
	    // this means that EOF is reached, 
	    // other side has closed connection
	    Log(DEBUG_LOG,LOG_UNIMP, tpparam.name, methodname << "Other side (" << *peer_addr << ") closed connection for socket " << conn_socket);
	    // disallow sending
	    AssocData* myassoc=const_cast<AssocData *>(receiver_thread_argp->peer_assoc);
	    if (myassoc->shutdown == false)
	    {
	      myassoc->shutdown= true;
	      if (shutdown(myassoc->socketfd,SHUT_WR))
	      {
		if ( errno != ENOTCONN )
		  Log(ERROR_LOG,LOG_UNIMP,tpparam.name, methodname << "shutdown (write) on socket " << conn_socket << " returned error:" << strerror(errno));
	      }
	    }
	    // not a real error, but we must quit the receive loop
	    recv_error= true;
	  }
	  else
	  {

	    Log(EVENT_LOG,LOG_UNIMP, tpparam.name, methodname << "<<--Received--<< packet (" << ret << " bytes) at socket " << conn_socket << " from " << *peer_addr);
	    // track number of received bytes
	    bytes_received+= ret;
	    buffer_bytes_left-= ret;
	  }
	}
      } // end if do not skip recv() statement      

      if (buffer_bytes_left < 0) ///< buffer space exhausted now
      {
	recv_error= true;
        Log(ERROR_LOG,LOG_CRIT, tpparam.name, methodname << "during receive buffer space exhausted");
      }

      if (!msgcontentlength_known) ///< common header not parsed
      {
	// enough bytes read to parse common header?
	if (bytes_received >= common_header_length)
	{
	  // get message content length in number of bytes
	  if (getmsglength(*netmsg, msgcontentlength))
	    msgcontentlength_known= true;
	  else
	  {
	    ERRCLog(tpparam.name, methodname << "Not a valid protocol header - discarding received packet. received size " << msgcontentlength);

	    ostringstream hexdumpstr;
	    netmsg->hexdump(hexdumpstr,netmsg->get_buffer(),bytes_received);
	    DLog(tpparam.name,"dumping received bytes:" << hexdumpstr.str());

	    // reset all counters
	    msgcontentlength= 0;
	    msgcontentlength_known= false;
	    bytes_received= 0;
	    pdu_complete= false;
	    continue;
	  }
	}
      } // endif common header not parsed

      // check whether we have read the whole Protocol PDU
      DLog(tpparam.name, "bytes_received-common_header_length=" << bytes_received-common_header_length << " msgcontentlength: " << msgcontentlength);
      if (msgcontentlength_known)
      {
	if (bytes_received-common_header_length >= msgcontentlength )
	{
	  pdu_complete= true;
	  // truncate buffer exactly at common_header_length+msgcontentlength==PDU size, trailing stuff is handled separately
	  netmsg->truncate(common_header_length+msgcontentlength);

	  // trailing bytes are copied into new buffer
	  if (bytes_received-common_header_length > msgcontentlength)
	  {
	    WLog(tpparam.name,"trailing bytes - received more bytes ("<<bytes_received<<") than expected for PDU (" << common_header_length+msgcontentlength << ")");
	    remainbuf= new NetMsg(NetMsg::max_size);
	    trailingbytes= (bytes_received-common_header_length) - msgcontentlength;
	    bytes_received= common_header_length+msgcontentlength;
	    memcpy(remainbuf->get_buffer(),netmsg->get_buffer()+common_header_length+msgcontentlength, trailingbytes);
	  }
	}
	else
	{ // not enough bytes in the buffer must continue to read from network
	  skiprecv= false;
	}
      } // endif msgcontentlength_known
    } // end while (!pdu_complete && !recv_error && !signalled for termination)
    // >>>>>>>>>>>>>>>>>>>>>>>>>>> while >>>>>>>>>>>>>>>>>>>>>>>>

    // if other side closed the connection, we should still be able to deliver the remaining data
    if (ret == 0)
    {
      recv_error= false;
    }

    // deliver only complete PDUs to signaling module
    if (!recv_error && pdu_complete)
    {
      // create TPMsg and send it to the signaling thread
      tpmsg = new(nothrow) TPMsg(netmsg, peer_addr->copy(), own_addr->copy());
      if (tpmsg)
      {
	DLog(tpparam.name, methodname << "receipt of PDU now complete, sending msg#" << tpmsg->get_id()
	     << " to module " <<  message::get_qaddr_name(tpparam.dest));
      }

      debug_pdu=false;

      if (debug_pdu)
      {
	ostringstream hexdump;
	netmsg->hexdump(hexdump,netmsg->get_buffer(),bytes_received);
	Log(DEBUG_LOG,LOG_NORMAL, tpparam.name,"PDU debugging enabled - Received:" << hexdump.str());
      }

      // send the message if it was successfully created
      // bool message::send_to(qaddr_t dest, bool exp = false);
      if (!tpmsg
	  || (!tpmsg->get_peeraddress())
	  || (!tpmsg->send(message::qaddr_tp_over_tcp, tpparam.dest))) 
      {
	Log(ERROR_LOG,LOG_NORMAL, tpparam.name, methodname << "Cannot allocate/send TPMsg");
	if (tpmsg) delete tpmsg;
      } // end if tpmsg not allocated or not addr or not sent
      

    } // end if !recv_error
    else
    { // error during receive or PDU incomplete
      if (bytes_received>0)
      {
	Log(WARNING_LOG,LOG_NORMAL, tpparam.name, methodname << "Attention! " << (recv_error? "Receive error, " : "") << (pdu_complete ?  "PDU complete" : "PDU incomplete") << "received bytes: " << bytes_received);
      }

      if (!pdu_complete && bytes_received>0 && bytes_received<common_header_length)
      {
	ostringstream hexdumpstr;
	netmsg->hexdump(hexdumpstr,netmsg->get_buffer(),bytes_received);
	Log(DEBUG_LOG,LOG_NORMAL,tpparam.name,"Message too short to be a valid protocol header - dumping received bytes:" << hexdumpstr.str());	
      }
      // leave the outer loop
      /**********************/
      break;
      /**********************/
    } // end else

  } // end while (thread not signalled for termination)

  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, methodname << "Thread <" << pthread_self() 
      << "> shutting down and closing socket " << receiver_thread_argp->peer_assoc->peer);

  // shutdown socket
  if (shutdown(conn_socket, SHUT_RD))
  {
    if ( errno != ENOTCONN )
      Log(ERROR_LOG,LOG_NORMAL, tpparam.name, methodname << "Thread <" << pthread_self() << "> shutdown (read) on socket failed, reason: " << strerror(errno));
  }

  // close socket
  close(conn_socket);

  receiver_thread_argp->terminated= true;
  //delete netmsg;

  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, methodname << "Thread <" << pthread_self() << "> terminated");

#ifdef _DEBUG
  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, methodname << "Signaling main loop for cleanup");
#endif
  // notify master thread for invocation of cleanup procedure
  TPoverTCPMsg* newmsg= new(nothrow)TPoverTCPMsg(receiver_thread_argp->peer_assoc);
  // send message to main loop thread
  newmsg->send_to(tpparam.source);

}  


/** this signals a terminate to a thread and wait for the thread to stop
 *  @note it is not safe to access any thread related data after this method returned,
 *        because the receiver thread will initiate a cleanup_receiver_thread() method 
 *        which may erase all relevant thread data.
 */
void 
TPoverTCP::stop_receiver_thread(AssocData* peer_assoc)
{
  // All operations on  recv_thread_argmap and connmap require an already acquired lock
  // after this procedure peer_assoc may be invalid because it was erased

  // start critical section

  if (peer_assoc == 0)
    return;

  pthread_t thread_id=  peer_assoc->thread_ID;
  
  // try to clean up receiver_thread_arg
  recv_thread_argmap_t::iterator recv_thread_arg_iter= recv_thread_argmap.find(thread_id);
  receiver_thread_arg_t* recv_thread_argp=  
    (recv_thread_arg_iter != recv_thread_argmap.end()) ? recv_thread_arg_iter->second : 0;
  if (recv_thread_argp)
  {
    if (!recv_thread_argp->terminated)
    {
      // thread signaled termination, but is not?
      Log(EVENT_LOG,LOG_NORMAL, tpparam.name,"stop_receiver_thread() - Receiver thread <" << thread_id << "> signaled for termination");

      // signal thread for its termination
      recv_thread_argp->sig_terminate= true;
      // wait for thread to join after termination
      pthread_join(thread_id, 0);
      // the dying thread will signal main loop to call this method, but this time we should enter the else branch
      return;
    }
  }
  else
    Log(ERROR_LOG,LOG_NORMAL, tpparam.name,"stop_receiver_thread() - Receiver thread <" << thread_id << "> not found");

}


/** cleans up left over structures (assoc,receiver_thread_arg) from already terminated receiver_thread
 *  usually called by the master_thread after the receiver_thread terminated
 * @note clean_up_receiver_thread() should be only called when an outer lock ensures that peer_assoc
 *       is still valid
 */
void 
TPoverTCP::cleanup_receiver_thread(AssocData* peer_assoc)
{
  // All operations on  recv_thread_argmap and connmap require an already acquired lock
  // after this procedure peer_assoc may be invalid because it was erased

  // start critical section

  if (peer_assoc == 0)
    return;

  pthread_t thread_id=  peer_assoc->thread_ID;
  
  // try to clean up receiver_thread_arg
  recv_thread_argmap_t::iterator recv_thread_arg_iter= recv_thread_argmap.find(thread_id);
  receiver_thread_arg_t* recv_thread_argp=  
    (recv_thread_arg_iter != recv_thread_argmap.end()) ? recv_thread_arg_iter->second : 0;
  if (recv_thread_argp)
  {
    if (!recv_thread_argp->terminated)
    {
      // thread signaled termination, but is not?
      Log(ERROR_LOG,LOG_NORMAL, tpparam.name,"cleanup_receiver_thread() - Receiver thread <" << thread_id << "> not terminated yet?!");
      return;
    }
    else
    { // if thread is already terminated
      Log(EVENT_LOG,LOG_NORMAL, tpparam.name,"cleanup_receiver_thread() - Receiver thread <" << thread_id << "> is terminated");

      // delete it from receiver map
      recv_thread_argmap.erase(recv_thread_arg_iter);

      // then delete receiver arg structure
      delete recv_thread_argp;
    }
  }

  // delete entry from connection map

  // cleanup sender thread
  // no need to lock explicitly, because caller of cleanup_receiver_thread() must already locked
  terminate_sender_thread(peer_assoc);

  // delete the AssocData structure from the connection map
  // also frees allocated AssocData structure
  connmap.erase(peer_assoc);

  // end critical section

  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name,"cleanup_receiver_thread() - Cleanup receiver thread <" << thread_id << ">. Done.");
}


/* sends a stop message to the sender thread that belongs to the peer address given in assoc
 * @note terminate_receiver_thread() should be only called when an outer lock ensures that assoc
 *       is still valid, a lock is also required, because senderthread_queuemap is changed
 */
void 
TPoverTCP::terminate_sender_thread(const AssocData* assoc)
{
  if (assoc == 0)
  {
    Log(ERROR_LOG,LOG_NORMAL,tpparam.name,"terminate_sender_thread() - assoc data == NULL");
    return;
  }

  sender_thread_queuemap_t::iterator it= senderthread_queuemap.find(assoc->peer);

  if (it != senderthread_queuemap.end())
  { // we have a sender thread: send a stop message to it
    FastQueue* destqueue= it->second; 
    if (destqueue)
    {
      TPoverTCPMsg* internalmsg= new TPoverTCPMsg(assoc,tpparam.source,TPoverTCPMsg::stop);
      if (internalmsg)
      {
	// send the internal message to the sender thread queue
	internalmsg->send(tpparam.source,destqueue);
      }
    }
    else
    {
      Log(WARNING_LOG,LOG_NORMAL,tpparam.name,"terminate_sender_thread() - found entry for address, but no sender thread. addr:" << assoc->peer);
    }
    // erase entry from map
    senderthread_queuemap.erase(it);
  }
}

/* terminate all active threads
 * note: locking should not be necessary here because this message is called as last method from
 * main_loop()
 */
void 
TPoverTCP::terminate_all_threads()
{
  AssocData* assoc= 0;
  receiver_thread_arg_t* terminate_argp;

  for (recv_thread_argmap_t::iterator terminate_iterator=  recv_thread_argmap.begin();
       terminate_iterator !=  recv_thread_argmap.end();
       terminate_iterator++)
  {
    if ( (terminate_argp= terminate_iterator->second) != 0)
    {
      // we need a non const pointer to erase it later on
      assoc= const_cast<AssocData*>(terminate_argp->peer_assoc);
      // check whether thread is still alive
      if (terminate_argp->terminated == false)
      {
	terminate_argp->sig_terminate= true;
	// then wait for its termination
	Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, 
	    "Signaled receiver thread <" << terminate_iterator->first << "> for termination");
	
	pthread_join(terminate_iterator->first, 0);
	
	Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, "Thread <" << terminate_iterator->first  << "> is terminated");
      }
      else
	Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, 
	    "Receiver thread <" << terminate_iterator->first << "> already terminated");
	
      // cleanup all remaining argument structures of terminated threads
      delete terminate_argp;

      // terminate any related sender thread that is still running
      terminate_sender_thread(assoc);
      
      connmap.erase(assoc);
      // delete assoc is not necessary, because connmap.erase() will do the job
    }
  } // end for
}


/**
 * sender thread starter: 
 * just a static starter method to allow starting the 
 * actual sender_thread() method.
 *
 * @param argp - pointer to the current TPoverTCP object instance and receiver_thread_arg_t struct
 */
void*
TPoverTCP::sender_thread_starter(void *argp)
{
  sender_thread_start_arg_t *sargp= static_cast<sender_thread_start_arg_t *>(argp);
  
  //cout << "invoked sender_thread_Starter" << endl;

  // invoke sender thread method
  if (sargp != 0 && sargp->instance != 0)
  {
    // call receiver_thread member function on object instance
    sargp->instance->sender_thread(sargp->sender_thread_queue);

    //cout << "Before deletion of sarg" << endl;

    // no longer needed
    delete sargp;
  }
  else
  {
    Log(ERROR_LOG,LOG_CRIT,"sender_thread_starter","while starting sender_thread: 0 pointer to arg or object");
  }
  return 0;
}




/**
 * receiver thread starter: 
 * just a static starter method to allow starting the 
 * actual receiver_thread() method.
 *
 * @param argp - pointer to the current TPoverTCP object instance and receiver_thread_arg_t struct
 */
void*
TPoverTCP::receiver_thread_starter(void *argp)
{
  receiver_thread_start_arg_t *rargp= static_cast<receiver_thread_start_arg_t *>(argp);
  // invoke receiver thread method
  if (rargp != 0 && rargp->instance != 0)
  {
    // call receiver_thread member function on object instance
    rargp->instance->receiver_thread(rargp->rtargp);

    // no longer needed
    delete rargp;
  }
  else
  {
    Log(ERROR_LOG,LOG_CRIT,"receiver_thread_starter","while starting receiver_thread: 0 pointer to arg or object");
  }
  return 0;
}


void
TPoverTCP::create_new_sender_thread(FastQueue* senderfqueue)
{
  Log(EVENT_LOG,LOG_NORMAL, tpparam.name, "Starting new sender thread...");

  pthread_t senderthreadid;
  // create new thread; (arg == 0) is handled by thread, too
  int pthread_status= pthread_create(&senderthreadid, 
				     NULL, // NULL: default attributes: thread is joinable and has a 
				     //       default, non-realtime scheduling policy
				     TPoverTCP::sender_thread_starter,
				     new sender_thread_start_arg_t(this,senderfqueue));
  if (pthread_status)
  {
    Log(ERROR_LOG,LOG_CRIT, tpparam.name, "A new thread could not be created: " <<  strerror(pthread_status));
    
    delete senderfqueue;
  }
}


void
TPoverTCP::create_new_receiver_thread(AssocData* peer_assoc)
{
  receiver_thread_arg_t* argp= 
    new(nothrow) receiver_thread_arg(peer_assoc);
  
  Log(EVENT_LOG,LOG_NORMAL, tpparam.name, "Starting new receiver thread...");

  // create new thread; (arg == 0) is handled by thread, too
  int pthread_status= pthread_create(&peer_assoc->thread_ID, 
				     NULL, // NULL: default attributes: thread is joinable and has a 
				     //       default, non-realtime scheduling policy
				     receiver_thread_starter,
				     new(nothrow) receiver_thread_start_arg_t(this,argp));
  if (pthread_status)
  {
    Log(ERROR_LOG,LOG_CRIT, tpparam.name, "A new thread could not be created: " <<  strerror(pthread_status));
    
    delete argp;
  }
  else
  {
    lock(); // install_cleanup_thread_lock(TPoverTCP, this);

    // remember pointer to thread arg structure
    // thread arg structure should be destroyed after thread termination only
    pair<recv_thread_argmap_t::iterator, bool> tmpinsiterator=
      recv_thread_argmap.insert( pair<pthread_t,receiver_thread_arg_t*> (peer_assoc->thread_ID,argp) );
    if (tmpinsiterator.second == false)
    {
      Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Thread argument could not be inserted into hashmap");
    }
    unlock(); // uninstall_cleanup(1);
  }  
}


/**
 * master listener thread starter: 
 * just a static starter method to allow starting the 
 * actual master_listener_thread() method.
 *
 * @param argp - pointer to the current TPoverTCP object instance
 */
void*
TPoverTCP::master_listener_thread_starter(void *argp)
{
  // invoke listener thread method
  if (argp != 0)
  {
    (static_cast<TPoverTCP*>(argp))->master_listener_thread();
  }
  return 0;
}



/**
 * master listener thread: waits for incoming connections at the well-known tcp port
 * when a connection request is received this thread spawns a receiver_thread for
 * receiving packets from the peer at the new socket. 
 */
void
TPoverTCP::master_listener_thread()
{
  // create a new address-structure for the listening masterthread
  struct sockaddr_in6 own_address_v6;
  own_address_v6.sin6_family = AF_INET6;
  own_address_v6.sin6_flowinfo= 0;
  own_address_v6.sin6_port = htons(tpparam.port); // use port number in param structure
  // accept incoming connections on all interfaces 
  own_address_v6.sin6_addr = in6addr_any;
  own_address_v6.sin6_scope_id= 0;

  // create a new address-structure for the listening masterthread
  struct sockaddr_in own_address_v4;
  own_address_v4.sin_family = AF_INET;
  own_address_v4.sin_port = htons(tpparam.port); // use port number in param structure
  // accept incoming connections on all interfaces
  own_address_v4.sin_addr.s_addr = INADDR_ANY;
  
  // create a listening socket
  int master_listener_socket= socket( AF_INET6, SOCK_STREAM, IPPROTO_TCP);
  if (master_listener_socket!=-1) v4_mode = false;
  if (master_listener_socket == -1) {
	  master_listener_socket= socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
	  if (master_listener_socket!=-1) v4_mode = true;
  }
  if (master_listener_socket == -1)
  {
    Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Could not create a new socket, error: " << strerror(errno));
    return;
  }
  
  // Disable Nagle Algorithm, set (TCP_NODELAY)
  int nodelayflag= 1;
  int status= setsockopt(master_listener_socket,
			 IPPROTO_TCP,
			 TCP_NODELAY,
			 &nodelayflag,
			 sizeof(nodelayflag));
  if (status)
  {
    Log(ERROR_LOG,LOG_NORMAL,tpparam.name, "Could not set socket option TCP_NODELAY:" << strerror(errno));
  }
  
  // Reuse ports, even if they are busy
  int socketreuseflag= 1;
  status= setsockopt(master_listener_socket,
			   SOL_SOCKET,
			   SO_REUSEADDR,
			   (const char *) &socketreuseflag,
			   sizeof(socketreuseflag));
  if (status)
  {
       Log(ERROR_LOG,LOG_NORMAL,tpparam.name, "Could not set socket option SO_REUSEADDR:" << strerror(errno));
  }
  
  
  // bind the newly created socket to a specific address
  int bind_status = bind(master_listener_socket, v4_mode ?
			 reinterpret_cast<struct sockaddr *>(&own_address_v4) :
			 reinterpret_cast<struct sockaddr *>(&own_address_v6),
					 v4_mode ? sizeof(own_address_v4) : sizeof(own_address_v6));
  if (bind_status)
    { 
      Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Binding to " 
	  <<  inet_ntop(AF_INET6, &own_address_v6.sin6_addr, in6_addrstr, INET6_ADDRSTRLEN)
	  << " port " << tpparam.port << " failed, error: " << strerror(errno));
      return;
    }

    // listen at the socket, 
    // queuesize for pending connections= max_listen_queue_size
    int listen_status = listen(master_listener_socket, max_listen_queue_size);
    if (listen_status)
    {
      Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Listen at socket " << master_listener_socket 
	  << " failed, error: " << strerror(errno));
      return;
    }
    else
    {
      Log(INFO_LOG,LOG_NORMAL, tpparam.name, color[green] << "Listening at port #" << tpparam.port << color[off]);
    }

    // activate O_NON_BLOCK for accept (accept does not block)
    fcntl(master_listener_socket,F_SETFL, O_NONBLOCK);

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
    int conn_socket;

    // check whether this thread is signaled for termination
    while(! (terminate= (currstate==STATE_ABORT || currstate==STATE_STOP) ) )
    {
      // wait on number_poll_sockets (main drm socket) 
      // for the events specified above for sleep_time (in ms)
      poll_status= poll(&poll_fd, number_poll_sockets, tpparam.sleep_time);
      if (poll_fd.revents & POLLERR) // Error condition
      {
	if (errno != EINTR) 
	{
	  Log(ERROR_LOG,LOG_CRIT, tpparam.name, 
	      "Poll caused error " << strerror(errno) << " - indicated by revents");
	}
	else
	{
	  Log(EVENT_LOG,LOG_NORMAL, tpparam.name, "poll(): " << strerror(errno));
	}

      }
      if (poll_fd.revents & POLLHUP) // Hung up 
      {
	Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Poll hung up");
	return;
      }
      if (poll_fd.revents & POLLNVAL) // Invalid request: fd not open
      {
	Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Poll Invalid request: fd not open");
	return;
      }
      
      switch (poll_status)
      {
	case -1:
	  if (errno != EINTR)
	  {
	    Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Poll status indicates error: " << strerror(errno));
	  }
	  else
	  {
	    Log(EVENT_LOG,LOG_NORMAL, tpparam.name, "Poll status: " << strerror(errno));
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

      // after a successful accept call, 
      // accept stores the address information of the connecting party
      // in peer_address and the size of its address in addrlen
      peer_address_len= sizeof(peer_address);
      conn_socket = accept (master_listener_socket,
			    reinterpret_cast<struct sockaddr *>(&peer_address),
			    &peer_address_len);
      if (conn_socket == -1)
      {
	if (errno != EWOULDBLOCK && errno != EAGAIN)
	{
	  Log(ERROR_LOG,LOG_EMERG, tpparam.name, "Accept at socket " << master_listener_socket
	      << " failed, error: " << strerror(errno));
	  return;
	}
      }
      else
      {
	// create a new assocdata-object for the new thread
	AssocData* peer_assoc = NULL;
	appladdress addr(peer_address, IPPROTO_TCP);

	Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, "<<--Received connect--<< request from " << addr.get_ip_str() 
	    << " port #" << addr.get_port());

	struct sockaddr_in6 own_address;
	if (v4_mode) {
		struct sockaddr_in own_address_v4;
		socklen_t own_address_len_v4 = sizeof(own_address_v4);
		getsockname(conn_socket, reinterpret_cast<struct sockaddr*>(&own_address_v4), &own_address_len_v4);
		v4_to_v6(&own_address_v4, &own_address);
	} else {
		socklen_t own_address_len= sizeof(own_address);
		getsockname(conn_socket, reinterpret_cast<struct sockaddr*>(&own_address), &own_address_len);
	}

	// AssocData will copy addr content into its own structure
	// allocated peer_assoc will be stored in connmap
	peer_assoc = new(nothrow) AssocData(conn_socket, addr, appladdress(own_address,IPPROTO_TCP));

	bool insert_success= false;
	if (peer_assoc)
	{
	  // start critical section
	  lock(); // install_cleanup_thread_lock(TPoverTCP, this);
	  insert_success= connmap.insert(peer_assoc);
	  // end critical section
	  unlock(); // uninstall_cleanup(1);
	}
	
	
	if (insert_success == false) // not inserted into connmap
	{
	  Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Cannot insert AssocData for socket " << conn_socket
	      << ", " << addr.get_ip_str() << ", port #" 
	      << addr.get_port() << " into connection map, aborting connection...");

	  // abort connection, delete its AssocData
	  close (conn_socket);
	  if (peer_assoc) 
	  { 
	    delete peer_assoc;
	    peer_assoc= 0;
	  }
	  return;
		
	} //end __else(connmap.insert());__
	
	// create a new thread for each new connection
	create_new_receiver_thread(peer_assoc);
      } // end __else (connsocket)__
      
      // get new thread state
      currstate= get_state();

    } // end while(!terminate)
    return;
} // end listen_for_connections()    


TPoverTCP::~TPoverTCP()
{
  init= false;
  this->connmap.clear();

  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name,  "Destructor called");

  QueueManager::instance()->unregister_queue(tpparam.source);
}

/** TPoverTCP Thread main loop.
 * This loop checks for internal messages of either
 * a send() call to start a new receiver thread, or,
 * of a receiver_thread() that signals its own termination
 * for proper cleanup of control structures.
 * It also handles the following internal TPoverTCPMsg types:
 * - TPoverTCPMsg::stop - a particular receiver thread is terminated
 * - TPoverTCPMsg::start - a particular receiver thread is started
 * @param nr number of current thread instance
 */
void 
TPoverTCP::main_loop(uint32 nr)
{

  // get internal queue for messages from receiver_thread
  FastQueue* fq = get_fqueue();
  if (!fq) 
  {
    Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Cannot find message queue");
    return;
  } // end if not fq
  // register queue for receiving internal messages from other modules
  QueueManager::instance()->register_queue(fq,tpparam.source);

  // start master listener thread
  pthread_t master_listener_thread_ID;
  int pthread_status= pthread_create(&master_listener_thread_ID, 
				     NULL, // NULL: default attributes: thread is joinable and has a 
				     //       default, non-realtime scheduling policy
				     master_listener_thread_starter,
				     this);
  if (pthread_status)
  {
    Log(ERROR_LOG,LOG_CRIT, tpparam.name, 
	"New master listener thread could not be created: " <<  strerror(pthread_status));
  }
  else
    Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, "Master listener thread started");


  // define max latency for thread reaction on termination/stop signal
  timespec wait_interval= { 0, 250000000L }; // 250ms
  message* internal_thread_msg = NULL;
  state_t currstate= get_state();

  // check whether this thread is signaled for termination
  while( currstate!=STATE_ABORT && currstate!=STATE_STOP )  
  {
    // poll internal message queue (blocking)
    if ( (internal_thread_msg= fq->dequeue_timedwait(wait_interval)) != 0 )
    {
      TPoverTCPMsg* internalmsg= dynamic_cast<TPoverTCPMsg*>(internal_thread_msg);
      if (internalmsg)
      {
	if (internalmsg->get_msgtype() == TPoverTCPMsg::stop)
	{
	  // a receiver thread terminated and signaled for cleanup by master thread
	  AssocData* assocd= const_cast<AssocData*>(internalmsg->get_peer_assoc());
	  Log(DEBUG_LOG,LOG_NORMAL, tpparam.name, "Got cleanup request for thread <" << assocd->thread_ID <<'>');
	  lock();
	  cleanup_receiver_thread( assocd );
	  unlock();
	}
	else
	if (internalmsg->get_msgtype() == TPoverTCPMsg::start)
	{
	  // start a new receiver thread
	  create_new_receiver_thread( const_cast<AssocData*>(internalmsg->get_peer_assoc()) );
	}
	else
	  Log(ERROR_LOG,LOG_CRIT, tpparam.name, "unexpected internal message:" << internalmsg->get_msgtype());
	  
	delete internalmsg;
      }
      else
      {
	Log(ERROR_LOG,LOG_CRIT, tpparam.name, "Dynamic_cast failed - received unexpected and unknown internal message source "
	    << internal_thread_msg->get_source());
      }
    } // endif

    // get thread state
    currstate= get_state();
  } // end while

  if (currstate==STATE_STOP)
  {
    // start abort actions
    Log(INFO_LOG,LOG_NORMAL, tpparam.name, "Asked to abort, stopping all receiver threads");
  } // end if stopped

  // do not accept any more messages
  fq->shutdown();
  // terminate all receiver and sender threads that are still active 
  terminate_all_threads();
}

} // end namespace protlib
///@}
