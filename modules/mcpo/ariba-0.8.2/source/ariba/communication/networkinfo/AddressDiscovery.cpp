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

#include "AddressDiscovery.h"
#include "ariba/config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <ifaddrs.h>

#ifdef HAVE_LIBBLUETOOTH
  #include <bluetooth/bluetooth.h>
  #include <bluetooth/hci.h>
  #include <bluetooth/hci_lib.h>
#endif

namespace ariba {
namespace communication {

mac_address AddressDiscovery::getMacFromIF( const char* name ) {
	mac_address addr;
#ifdef HAVE_LIBBLUETOOTH
	int s;
	struct ifreq buffer;
	s = socket(PF_INET, SOCK_DGRAM, 0);
	memset(&buffer, 0x00, sizeof(buffer));
	strcpy(buffer.ifr_name, name);
	ioctl(s, SIOCGIFHWADDR, &buffer);
	close(s);
	addr.assign( (uint8_t*)buffer.ifr_hwaddr.sa_data, 6 );
#endif
	return addr;
}

int AddressDiscovery::dev_info(int s, int dev_id, long arg) {
#ifdef HAVE_LIBBLUETOOTH
	endpoint_set* set = (endpoint_set*)arg;
	struct hci_dev_info di;
	memset(&di, 0, sizeof(struct hci_dev_info));
	di.dev_id = dev_id;
	if (ioctl(s, HCIGETDEVINFO, (void *) &di)) return 0;
	mac_address mac;
	mac.bluetooth( di.bdaddr );
	address_vf vf = mac;
	set->add(vf);
#endif
	return 0;
}

void AddressDiscovery::discover_bluetooth( endpoint_set& endpoints ) {
#ifdef HAVE_LIBBLUETOOTH
	hci_for_each_dev(HCI_UP, &AddressDiscovery::dev_info, (long)&endpoints );
#endif
}

void AddressDiscovery::discover_ip_addresses( endpoint_set& endpoints ) {
	struct ifaddrs* ifaceBuffer = NULL;
	void*           tmpAddrPtr  = NULL;

	int ret = getifaddrs( &ifaceBuffer );
	if( ret != 0 ) return;

	for( struct ifaddrs* i=ifaceBuffer; i != NULL; i=i->ifa_next ) {

		// ignore devices that are disabled or have no ip
		if(i == NULL) continue;
		struct sockaddr* addr = i->ifa_addr;
		if (addr==NULL) continue;

		// ignore tun devices
		string device = string(i->ifa_name);
		if(device.find_first_of("tun") == 0) continue;

		if (addr->sa_family == AF_INET) {
			// look for ipv4
			char straddr[INET_ADDRSTRLEN];
			tmpAddrPtr= &((struct sockaddr_in*)addr)->sin_addr;
			inet_ntop( i->ifa_addr->sa_family, tmpAddrPtr, straddr, sizeof(straddr) );
			ip_address ip = straddr;
			if (ip.is_loopback()) continue;
			address_vf vf = ip;
			endpoints.add( vf );
		} else
		if (addr->sa_family == AF_INET6) {
			// look for ipv6
			char straddr[INET6_ADDRSTRLEN];
			tmpAddrPtr= &((struct sockaddr_in6*)addr)->sin6_addr;
			inet_ntop( i->ifa_addr->sa_family, tmpAddrPtr, straddr, sizeof(straddr) );
			ip_address ip = straddr;
			if (ip.is_loopback()) continue;
//			if (ip.is_link_local()) continue;
			address_vf vf = ip;
			endpoints.add( vf );
		}
	}

	freeifaddrs(ifaceBuffer);
}

void AddressDiscovery::discover_endpoints( endpoint_set& endpoints ) {
	discover_ip_addresses( endpoints );
	discover_bluetooth( endpoints );
}

}} // namespace ariba, communication
