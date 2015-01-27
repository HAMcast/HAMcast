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

#include "NetworkInformation.h"
#include "ariba/config.h"

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
  #include <bluetooth/bluetooth.h>
  #include <bluetooth/hci.h>
  #include <bluetooth/hci_lib.h>
#endif

namespace ariba {
namespace communication {

use_logging_cpp(NetworkInformation);

NetworkInformation::NetworkInformation() : infoSocket( -1 ){

	infoSocket = socket( AF_INET, SOCK_DGRAM, 0 );
}

NetworkInformation::~NetworkInformation(){

	close( infoSocket );
}

NetworkInterfaceList NetworkInformation::getInterfaces(){

	NetworkInterfaceList retlist;

	//
	// get all network interfaces with basic information
	// and all addresses configured on the interface
	//

	// TODO: can be optimized using SIOCGIFCOUNT
	// but maybe this way its more robust?
	// TODO: or using if_nameindex() problem is here,
	// that i don't know the number of ifaces returned ...

	struct ifconf ifconf;
	int ifnum = 5;
	int lastlen = 0;

	ifconf.ifc_buf = NULL;
	ifconf.ifc_len = 0;

	while( true ){

		ifconf.ifc_len = sizeof(struct ifreq) * ifnum;
		ifconf.ifc_buf = (char*)realloc( ifconf.ifc_buf, ifconf.ifc_len );

		int ret = ioctl( infoSocket, SIOCGIFCONF, &ifconf );
		if (ret < 0){
			logging_error( "getting interface list failed with: " <<
						strerror(errno));

			// if the socket is bogus, try to get
			// a new one for the next call
			if(errno == EBADF){
				close( infoSocket );
				infoSocket = socket( AF_INET, SOCK_DGRAM, 0 );
			}

			return retlist;
		}

		if( ifconf.ifc_len > lastlen ){
			lastlen = ifconf.ifc_len;
			ifnum += 10;
			continue;
		}

		break; // length did not change any more, success
	}

	struct ifreq* ifr = ifconf.ifc_req;

	for (int i = 0; i<ifconf.ifc_len; i+=sizeof(struct ifreq), ifr++){
		NetworkInterface interface;
		interface.name = string( ifr->ifr_name );
		retlist.push_back( interface );
	}

	free( ifconf.ifc_buf );

	//
	// SIOCGIFCONF does not return interfaces that are currently
	// not running. therefore we try to complete the list of interfaces now
	// using getifaddrs. but we can't use _only_ getifaddrs, because
	// it only contains an interface it it has an address configured
	//

	struct ifaddrs* ifap;
	getifaddrs( &ifap );

	for( struct ifaddrs* p = ifap; p != NULL; p=p->ifa_next ){
		NetworkInterface interface;
		interface.name = string( p->ifa_name );

		if( find(retlist.begin(), retlist.end(), interface) == retlist.end() )
			retlist.push_back( interface );
	}

	freeifaddrs( ifap );

	//
	// now we start to complete the interface information.
	// we can't just call al IO ctrls and then get the
	// information, as some vars of the ifreq structure
	// hold several values, depending on the request
	// see /usr/include/net/if.h
	//

	NetworkInterfaceList::iterator i = retlist.begin();
	NetworkInterfaceList::iterator iend = retlist.end();

	for( ; i != iend; i++ ){

		NetworkInterface* interface = &(*i);
		struct ifreq ifr;

		memset( &ifr, 0, sizeof(struct ifreq) );
		strcpy( ifr.ifr_name, i->name.c_str() );


		{ // get interface index
			if( ioctl(infoSocket, SIOCGIFINDEX, &ifr) ){
				logging_error( "could not get interface index for " <<
						i->name << ": " << strerror(errno) );
				return retlist;
			}

			interface->index = ifr.ifr_ifindex;
		}

		{ // get interface flags
			if( ioctl(infoSocket, SIOCGIFFLAGS, &ifr) ){
				logging_error( "could not get interface flags for " <<
						i->name << ": " << strerror(errno) );
				return retlist;
			}

			interface->isRunning   = (ifr.ifr_flags & IFF_RUNNING)   != 0 ? true : false;
			interface->isUp        = (ifr.ifr_flags & IFF_UP)        != 0 ? true : false;
			interface->isLoopback  = (ifr.ifr_flags & IFF_LOOPBACK)  != 0 ? true : false;
			interface->isBroadcast = (ifr.ifr_flags & IFF_BROADCAST) != 0 ? true : false;
			interface->isMulticast = (ifr.ifr_flags & IFF_MULTICAST) != 0 ? true : false;
		}

		{ // get mtu
			if( ioctl(infoSocket, SIOCGIFMTU, &ifr) ){
				logging_error( "could not get mtu for " <<
						i->name << ": " << strerror(errno) );
				return retlist;
			}

			interface->mtu = ifr.ifr_mtu;
		}

		{ // get tx queue length
			if( ioctl(infoSocket, SIOCGIFTXQLEN, &ifr) ){
				logging_error( "could not get tx queue length for " <<
						i->name << ": " << strerror(errno) );
				return retlist;
			}

			interface->txQueueLen = ifr.ifr_qlen;
		}

	} // for( ; i != iend; i++ )*/

	//
	// not we try to get bluetooth interfaces
	//

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	int btsock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if(btsock <  0){
		logging_error("failed getting bluetooth raw socket");
		return retlist;
	}

	struct hci_dev_list_req* btlist = NULL;
	struct hci_dev_req* btdev = NULL;

	btlist = (hci_dev_list_req*)malloc(HCI_MAX_DEV *
			sizeof(struct hci_dev_list_req) + sizeof(struct hci_dev_req));

	btlist->dev_num = HCI_MAX_DEV;
	btdev = btlist->dev_req;

	if(ioctl(btsock, HCIGETDEVLIST, btlist) < 0){
		logging_error("failed getting requesting bluetooth devices");
		free(btlist);
		close(btsock);
		return retlist;
	}

	btdev = btlist->dev_req;

	for(int i=0; i<btlist->dev_num; i++, btdev++){
		struct hci_dev_info di;
		NetworkInterface interface;

		if(hci_devinfo(btdev->dev_id, &di) < 0) continue;
		if(hci_test_bit(HCI_RAW, &di.flags)) continue;

		interface.name = string(di.name);
		interface.index = di.dev_id;
		interface.mtu = di.sco_mtu;
		interface.isBroadcast = false;
		interface.isLoopback = false;
		interface.isMulticast = false;
		interface.isUp = hci_test_bit(HCI_UP, &di.flags);
		interface.isRunning = hci_test_bit(HCI_RUNNING, &di.flags);

		retlist.push_back( interface );
	}

	free(btlist);
	close(btsock);
#endif

	return retlist;
}

NetworkInterface NetworkInformation::getInterface(int index){

	NetworkInterfaceList ifaces = getInterfaces();
	NetworkInterfaceList::iterator i = ifaces.begin();
	NetworkInterfaceList::iterator iend = ifaces.end();

	for( ; i != iend; i++ ){
		if( i->index == index ) return *i;
	}

	return NetworkInterface::UNDEFINED;
}

NetworkInterface NetworkInformation::getInterface(string name){

	NetworkInterfaceList ifaces = getInterfaces();
	NetworkInterfaceList::iterator i = ifaces.begin();
	NetworkInterfaceList::iterator iend = ifaces.end();

	for( ; i != iend; i++ ){
		if( i->name.compare(name) == 0 ) return *i;
	}

	return NetworkInterface::UNDEFINED;
}

}} // namespace ariba, communication
