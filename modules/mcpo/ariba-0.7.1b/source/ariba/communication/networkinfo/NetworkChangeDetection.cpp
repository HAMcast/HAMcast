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

#include "NetworkChangeDetection.h"

namespace ariba {
namespace communication {

use_logging_cpp(NetworkChangeDetection);
SystemEventType NetworkChangeDetectionEventType("NetworkChangeDetectionEventType");

NetworkChangeDetection::NetworkChangeDetection()
	: running( false ), monitoringThread( NULL ), routingSocket( -1 ){
	startMonitoring();
}

NetworkChangeDetection::~NetworkChangeDetection(){
	stopMonitoring();
}

void NetworkChangeDetection::registerNotification(NetworkChangeInterface* callback){

	RegistrationList::iterator i = find(
		registrations.begin(), registrations.end(), callback );

	if( i == registrations.end() )
		registrations.push_back( callback );
}

void NetworkChangeDetection::unregisterNotification(NetworkChangeInterface* callback){

	RegistrationList::iterator i = find(
		registrations.begin(), registrations.end(), callback );

	if( i != registrations.end() )
		registrations.erase( i );
}

void NetworkChangeDetection::startMonitoring(){

	logging_debug( "starting network change monitoring ..." );

	running = true;
	monitoringThread = new boost::thread(
		boost::bind(&NetworkChangeDetection::monitoringThreadFunc, this) );
}

void NetworkChangeDetection::stopMonitoring(){

	logging_debug( "stopping network change monitoring ..." );

	// trigger the blocking read to end
	running = false;

	shutdown( routingSocket, SHUT_RDWR );
	close( routingSocket );

	// end the monitoring thread
	monitoringThread->join();

	delete monitoringThread;
	monitoringThread = NULL;
}

void NetworkChangeDetection::monitoringThreadFunc(NetworkChangeDetection* obj){

	//
	// Most information about this routing socket interface is available
	// when searching for NETLINK_ROUTE and in http://www.faqs.org/rfcs/rfc3549.html
	// A small program that uses this stuff is ifplugd, nice reference for
	// getting network interface notifications using routing sockets
	// (e.g. http://ifplugd.sourcearchive.com/documentation/0.26/ifmonitor_8c-source.html)
	// And the best resource is the linux code ... http://www.srcdoc.com/linux_2.2.26/rtnetlink_8h-source.html
	// More:
	//	http://m.linuxjournal.com/article/8498
	// 	http://lkml.indiana.edu/hypermail/linux/net/0508.2/0016.html
	//	http://infrahip.hiit.fi/hipl/release/1.0.1/hipl.1.0.1/hipd/netdev.c
	// 	http://www.google.de/codesearch?hl=de&q=rtmsg_ifinfo+show:Ndy7Mq7IdCw:aylASy5mtF0:ClfaxlLp-PU&source=universal&cs_p=ftp://ftp.kernel.org/pub/linux/kernel/v2.4/linux-2.4.18.tar.gz&cs_f=linux/net/core/rtnetlink.c#l490
	//	http://www.google.com/codesearch?hl=en&q=RTM_NEWADDR+NETLINK_ROUTE+show:eUekvGbF94M:iCwmLwtnVbU:38qoeYp--0A&sa=N&cd=1&ct=rc&cs_p=ftp://ftp.stacken.kth.se/pub/arla/arla-0.42.tar.gz&cs_f=arla-0.42/lib/roken/getifaddrs.c
	//	http://www.google.com/codesearch?hl=en&q=RTMGRP_LINK+rtm_newaddr+nl_groups+show:sUV8mV--Lb4:gk0jx_xv0Yg:ZB9MN7Fgkgg&sa=N&cd=1&ct=rc&cs_p=http://freshmeat.net/redir/strongswan/60428/url_bz2/strongswan-4.1.2.tar.bz2&cs_f=strongswan-4.1.4/src/charon/kernel/kernel_interface.c#l689
	//

	//
	// open the netlink routing socket, don't need raw sockets here!
	//

	obj->routingSocket = socket( PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE );
	if( obj->routingSocket < 0 ){
		logging_error("could not connect to routing socket: " +
					string(strerror(errno)));
		return;
	}

	//
	// set a socket read timeout. this is actually bad, but closing
	// the blocking socket using shutdown, close, or writing EOF to
	// the socket did not work. this seems to be a bug related to
	// routing sockets that will not close when in blocking mode
	//

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	setsockopt( obj->routingSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );

	//
	// bind the netlink routing socket to our local address
	// for the nl_groups, see http://www.cs.fsu.edu/~baker/devices/lxr/http/source/linux/include/linux/rtnetlink.h#L520
	// we listen on ipv4/6 new/delete address and link up/down notifications
	//

	struct sockaddr_nl addr;
	memset( &addr, 0, sizeof(addr) );
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_LINK;
	addr.nl_pid    = getpid();

	int ret = bind( obj->routingSocket, (struct sockaddr*)&addr, sizeof(addr) );
	if( ret < 0 ){
		close( obj->routingSocket );
		logging_error( "could not bind routing socket: " + string(strerror(errno)) );
		return;
	}

	//
	// read from the netlink routing socket
	//

	logging_debug( "network change monitoring started" );

	while( obj->running ){

		char buffer[1024];
		struct nlmsghdr* header = (struct nlmsghdr*)buffer;

		int bytesRead = recv( obj->routingSocket, &buffer, sizeof(buffer), 0 );

		if( bytesRead < 0 ){

			// socket timeout, just continue
			if( errno == EWOULDBLOCK ) continue;

			// triggered by shutdown() function in stopMonitoring
			if( errno == EBADF ) return;

			// gracefull exit by signals
			if( errno == EAGAIN || errno == EINTR ) return;

			// all others are some kind of error
			logging_error( "could not read from routing socket: " +
							string(strerror(errno)) );
			break;
		}

		for( ; bytesRead > 0; header = NLMSG_NEXT(header, bytesRead)) {
			if (!NLMSG_OK(header, (size_t)bytesRead) ||
				(size_t) bytesRead < sizeof(struct nlmsghdr) ||
				(size_t) bytesRead < (size_t)header->nlmsg_len) {
				continue;
			}

			// somehow all notifications with pid=0 are
			// invalid and occur much too often
			if(header->nlmsg_pid == 0) continue;

			//
			// here we evaluate the routing netlink message. for, the following
			// messages are of interest:
			//	RTM_NEWADDR, RTM_DELADDR -> new address on iface, address deleted from iface
			//

			NetworkChangeInterface::NetworkChangeInfo changeInfo;

			switch( header->nlmsg_type ){

				//
				// handle network address configuration changes
				// --> network layer notifications
				//

				case RTM_NEWADDR:
				case RTM_DELADDR:

					changeInfo = obj->extractAddressEvent( header );
					break;

				//
				// other RTM message are ignored in case
				// we did subscribe using nl_groups (see above)
				//

				default: break;

			} // switch( header->nlmsg_type )

			logging_info( "network change detected: [" << changeInfo << "]" );

			// put noficiation event into the system queue for main
			// thread synchronization. will be handled in NetworkChangeDetection::handleSystemEvent()
			// that will run synchronized with the main thread

			SystemQueue::instance().scheduleEvent(
				SystemEvent( obj, NetworkChangeDetectionEventType,
					new NetworkChangeInterface::NetworkChangeInfo(changeInfo)));

		} // for( ; bytesRead > 0; header = NLMSG_NEXT(header, bytesRead))
	} // while( running )

	logging_debug( "network change monitoring stopped" );
}

NetworkChangeInterface::NetworkChangeInfo NetworkChangeDetection::extractAddressEvent(struct nlmsghdr* header){
	NetworkChangeInterface::NetworkChangeInfo changeInfo;

	//
	// handle network address configuration changes
	// addresses are added/removed from an interface
	//

	if( header->nlmsg_type == RTM_NEWADDR ){
		logging_debug("network change message RTM_NEWADDR");
		changeInfo.type = NetworkChangeInterface::EventTypeInterfaceUp;
	}else if( header->nlmsg_type == RTM_DELADDR ){
		logging_debug("network change message RTM_DELADDR");
		changeInfo.type = NetworkChangeInterface::EventTypeInterfaceDown;
	}

	struct ifaddrmsg* detailAddr = (struct ifaddrmsg*)NLMSG_DATA(header);
	changeInfo.interface = networkInformation.getInterface( detailAddr->ifa_index );

	return changeInfo;
}

void NetworkChangeDetection::handleSystemEvent(const SystemEvent& event){

	NetworkChangeInterface::NetworkChangeInfo* changeInfo = event.getData<NetworkChangeInterface::NetworkChangeInfo>();
	const NetworkChangeDetection* obj = (const NetworkChangeDetection*)event.getListener();

	RegistrationList::const_iterator i = obj->registrations.begin();
	RegistrationList::const_iterator iend = obj->registrations.end();

	for( ; i != iend; i++ )
		(*i)->onNetworkChange(*changeInfo);

	delete changeInfo;
}

}} // namespace ariba, communication
