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

#include "BluetoothSdp.h"
#include "ariba/overlay/OverlayBootstrap.h"

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

// Attribute descriptors for SDP
// base was chosen randomly
#define SDP_SPOVNET_BASE 			0x4000
#define SDP_ATTR_SPOVNET_NAME		0x0000 + SDP_SPOVNET_BASE
#define SDP_ATTR_SPOVNET_INFO1		0x0001 + SDP_SPOVNET_BASE
#define SDP_ATTR_SPOVNET_INFO2		0x0002 + SDP_SPOVNET_BASE
#define SDP_ATTR_SPOVNET_INFO3		0x0003 + SDP_SPOVNET_BASE

// The SpoVNet unique identifier, this should be the same for all SpoVNet implementations
const uint8_t svc_uuid_int[] = {0x59, 0x29, 0x24, 0x34, 0x69, 0x42, 0x11, 0xde, 0x94,
		0x3e, 0x00, 0x21, 0x5d, 0xb4, 0xd8, 0x54};

const char *service_name = "SpoVNet";
const char *svc_dsc = "www.ariba-underlay.org";
const char *service_prov = "ITM Uni Karlsruhe";

#endif // HAVE_BLUETOOTH_BLUETOOTH_H


namespace ariba {
namespace utility {

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
static bdaddr_t bd_addr_any = {{0, 0, 0, 0, 0, 0}};
static bdaddr_t bd_addr_local = {{0, 0, 0, 0xff, 0xff, 0xff}};
#endif

use_logging_cpp(BluetoothSdp);
OverlayBootstrap* BluetoothSdp::CONNECTION_CHECKER = NULL;

BluetoothSdp::BluetoothSdp(BootstrapInformationCallback* _callback, string info)
	: BootstrapModule(_callback), scan_timer_(io_service_) {
	srand( time(NULL) );
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	// This can be ignored, as the channel we really be saved in one
	// of the info strings (as an attribute)
	channel_ = 1;
#endif // HAVE_BLUETOOTH_BLUETOOTH_H
}

BluetoothSdp::~BluetoothSdp() {
}

string BluetoothSdp::getName() {
	return "BluetoothSdp";
}

string BluetoothSdp::getInformation() {
	return "bootstrap module based on bluetooth service discovery protocol";
}

bool BluetoothSdp::isFunctional() {
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H
	return true;
#else
	return false;
#endif
}

void BluetoothSdp::start() {
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	/*
	 * Initializes and forks the scanner.
	 */

	io_service_.post(boost::bind(&BluetoothSdp::bt_scan, this));
	t_ = boost::thread(boost::bind(&boost::asio::io_service::run, &io_service_));

#endif // HAVE_BLUETOOTH_BLUETOOTH_H
}

void BluetoothSdp::stop() {
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	/*
	 * Stops the scanner.
	 */

	// not sure if this is thread safe
	io_service_.stop();
	t_.join();

	if(sdp_session_ != NULL)
		sdp_close(sdp_session_);

#endif // HAVE_BLUETOOTH_BLUETOOTH_H
}

void BluetoothSdp::publishService(string name, string info1, string info2,
		string info3) {
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	/*
	 * Publishes an SpoVNet SDP Service and
	 * adds the arguments as info attributes.
	 */

	logging_debug("registering SDP service");

	uint8_t rfcomm_channel = channel_;

	uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid, svc_class_uuid;
	sdp_list_t *l2cap_list = 0, *rfcomm_list = 0, *root_list = 0, *proto_list =
		0, *access_proto_list = 0, *svc_class_list = 0, *profile_list = 0;
	sdp_data_t *channel = 0;
	sdp_profile_desc_t profile;
	sdp_record_t record = {0};
	sdp_session_ = 0;

	if((name.length() > 256) || (info1.length() > 256) || (info2.length() > 256) || (info3.length() > 256)) {
		logging_error("string argument too long, max size is 256");
		return;
	}

	// prepare the info attribute buffers
	//string namebuf, info1buf, info2buf, info3buf;
	uint8_t namelen, info1len, info2len, info3len;

	namelen = (uint8_t)name.length();
	info1len = (uint8_t)info1.length();
	info2len = (uint8_t)info2.length();
	info3len = (uint8_t)info3.length();

	// set the general service ID
	sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
	sdp_set_service_id(&record, svc_uuid);

	// set the service class
	sdp_uuid16_create(&svc_class_uuid, SERIAL_PORT_SVCLASS_ID);
	svc_class_list = sdp_list_append(0, &svc_class_uuid);
	sdp_set_service_classes(&record, svc_class_list);

	// set the Bluetooth profile information
	sdp_uuid16_create(&profile.uuid, SERIAL_PORT_PROFILE_ID);
	profile.version = 0x0100;
	profile_list = sdp_list_append(0, &profile);
	sdp_set_profile_descs(&record, profile_list);

	// make the service record publicly browsable
	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root_list = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(&record, root_list);

	// set l2cap informatiint argc, char* argv[]on
	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	l2cap_list = sdp_list_append(0, &l2cap_uuid);
	proto_list = sdp_list_append(0, l2cap_list);

	// register the RFCOMM channel for RFCOMM sockets
	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
	rfcomm_list = sdp_list_append(0, &rfcomm_uuid);
	sdp_list_append(rfcomm_list, channel);
	sdp_list_append(proto_list, rfcomm_list);

	access_proto_list = sdp_list_append(0, proto_list);
	sdp_set_access_protos(&record, access_proto_list);

	// set the name, provider, and description
	sdp_set_info_attr(&record, service_name, service_prov, svc_dsc);

	// add the spovnet attributes
	sdp_attr_add_new(&record, SDP_ATTR_SPOVNET_NAME, SDP_TEXT_STR8,
			name.data());

	sdp_attr_add_new(&record, SDP_ATTR_SPOVNET_INFO1, SDP_TEXT_STR8,
			info1.data());

	sdp_attr_add_new(&record, SDP_ATTR_SPOVNET_INFO2, SDP_TEXT_STR8,
			info2.data());

	sdp_attr_add_new(&record, SDP_ATTR_SPOVNET_INFO3, SDP_TEXT_STR8,
			info3.data());

	// connect to the local SDP server, register the service record
	if( sdp_session_ == NULL ){
		sdp_session_ = sdp_connect(&bd_addr_any, &bd_addr_local, SDP_RETRY_IF_BUSY);
	}

	if (sdp_session_ == NULL) {
		logging_error( "something is wrong with your SDP server, nothing registered: " << strerror(errno) );
	} else {
		int ret = sdp_record_register(sdp_session_, &record, 0);

		if(ret < 0){
			logging_error("failed registering sdp record: " << strerror(errno));
		}else{
			logging_debug("sdp record registered using session " << sdp_session_);
		}
	}

	// cleanup
	sdp_data_free(channel);
	sdp_list_free(l2cap_list, 0);
	sdp_list_free(rfcomm_list, 0);
	sdp_list_free(root_list, 0);
	sdp_list_free(access_proto_list, 0);
	sdp_list_free(svc_class_list, 0);
	sdp_list_free(profile_list, 0);

#endif // HAVE_BLUETOOTH_BLUETOOTH_H
}

void BluetoothSdp::revokeService(string name) {
#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

	logging_debug("unregistering SDP service");
	sdp_close(sdp_session_);

#endif // HAVE_BLUETOOTH_BLUETOOTH_H
}

#ifdef HAVE_BLUETOOTH_BLUETOOTH_H

void BluetoothSdp::bt_scan() {

	//
	// scan for devices if we have no active rfcomm connections running.
	// otherwise we would break existing connections due to chipping seq
	//

	if(!haveConnections()){

		/*
		 * Scans for other bluetooth devices and starts a SDP search on them.
		 */

		logging_debug("scanning for peers");

		inquiry_info *ii = NULL;
		int max_rsp, num_rsp;
		int dev_id, sock, len, flags;
		int i;

		bdaddr_t address;
//		uint8_t channel;

		dev_id = hci_get_route(NULL);
		sock = hci_open_dev(dev_id);
		if (dev_id < 0 || sock < 0) {
			logging_error("opening socket for device "
					<< dev_id << " failed. can not scan for peers: " << strerror(errno));
			return;
		}

		len = 8;
		max_rsp = 255;
		flags = IREQ_CACHE_FLUSH;
		ii = (inquiry_info*) malloc(max_rsp * sizeof(inquiry_info));

		num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
		if (num_rsp < 0)
			logging_error("hci_inquiry failed with " << num_rsp << ": " << strerror(errno));

		for (i = 0; i < num_rsp; i++) {
			address = (ii + i)->bdaddr;

			string saddress = ba2string(&address);
			string sname = ba2name(&address, sock);

			logging_debug("found peer [" << saddress << "] [" << sname << "]");
			sdp_search( address, sname );
		}

		free(ii);
		close(sock);

	} else {
		logging_debug("have active connections, no sdp searching");
	}

	int nextscan = (rand() % 10) + 5;
	logging_debug("next sdp scan try in " << nextscan << " seconds");

	scan_timer_.expires_from_now( boost::posix_time::seconds(nextscan) );
	scan_timer_.async_wait( boost::bind(&BluetoothSdp::bt_scan, this) );
}

void BluetoothSdp::sdp_search(bdaddr_t target, string devicename) {

	/*
	 * Searches target for SDP records with the SpoVnet uuid
	 * and extracts its info attributes.
	 */

	int status;
	uuid_t svc_uuid;
	sdp_list_t *response_list, *search_list, *attrid_list;
	sdp_session_t *session = NULL;
	uint32_t range = 0x0000ffff;
	uint8_t port = 0;

	// connect to the SDP server running on the remote machine
	logging_debug("querying services from bt device ["
			<< ba2string(&target) << "] [" << devicename << "]");

	// prepare the buffers for the attributes
	char name[256], info1[256], info2[256], info3[256];

	session = sdp_connect(&bd_addr_any, &target, SDP_RETRY_IF_BUSY);

	if (session == NULL) {
		logging_error("failed to connect to SDP server at "
				<< ba2string(&target) << ": " << strerror(errno));
		return;
	}

	sdp_uuid128_create(&svc_uuid, &svc_uuid_int);
	search_list = sdp_list_append(0, &svc_uuid);
	attrid_list = sdp_list_append(0, &range);

	// get a list of service records that have UUID uuid_
	response_list = NULL;
	status = sdp_service_search_attr_req(session, search_list,
			SDP_ATTR_REQ_RANGE, attrid_list, &response_list);

	if (status == 0) {
		sdp_list_t *proto_list = NULL;
		sdp_list_t *r = response_list;

		// go through each of the service records
		for ( ; r != NULL; r = r->next) {
			sdp_record_t *rec = (sdp_record_t*) r->data;

			// get a list of the protocol sequences
			if (sdp_get_access_protos(rec, &proto_list) == 0) {

				// get the RFCOMM port number
				port = sdp_get_proto_port(proto_list, RFCOMM_UUID);

				sdp_list_free(proto_list, 0);

				sdp_get_string_attr(rec, SDP_ATTR_SPOVNET_NAME, (char*)&name, 256);
				sdp_get_string_attr(rec, SDP_ATTR_SPOVNET_INFO1, (char*)&info1, 256);
				sdp_get_string_attr(rec, SDP_ATTR_SPOVNET_INFO2, (char*)&info2, 256);
				sdp_get_string_attr(rec, SDP_ATTR_SPOVNET_INFO3, (char*)&info3, 256);

				logging_info("Remote peer name is: " << name);
				logging_info("Remote peer info1 is: " << info1);
				logging_info("Remote peer info2 is: " << info2);
				logging_info("Remote peer info3 is: " << info3);

				// Callback
				callback->onBootstrapServiceFound(name, info1, info2, info3);
			}
			sdp_record_free(rec);
		}
	} else {
		logging_error("sdp_service_search_attr_req failed with timeout: " << strerror(errno));
	}

	sdp_list_free(response_list, 0);
	sdp_list_free(search_list, 0);
	sdp_list_free(attrid_list, 0);
	sdp_close(session);
}

string BluetoothSdp::ba2string(bdaddr_t* ba) {
	/*
	 * Returns a string holding the bt adress in human readable form.
	 */
	char str[32] = { 0 };
	ba2str(ba, str);
	string result = str;
	return result;
}

string BluetoothSdp::ba2name(bdaddr_t* ba, int sock){

	char name[256] = {0};
	memset(name, 0, sizeof(name));

	if( hci_read_remote_name(sock, ba, sizeof(name), name, 0) < 0 )
		strcpy(name, "unknown");

	string result = name;
	return result;
}

bool BluetoothSdp::haveConnections(){

	// TODO: currently we check for overlay connectivity

	if(CONNECTION_CHECKER == NULL) return false;
	return CONNECTION_CHECKER->haveOverlayConnections();


	/* TODO: this will check for rfcomm connections
	struct hci_conn_list_req* cl = NULL;
	struct hci_conn_info* ci = NULL;

	int btsock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if(btsock <  0){
		logging_error("failed getting bluetooth raw socket");
		return true; // return true to be safe here and not perform sdp scan
	}

	cl = (struct hci_conn_list_req*)malloc(10 * sizeof(struct hci_conn_info) + sizeof(struct hci_conn_list_req));

	cl->dev_id = hci_get_route(NULL);;
	cl->conn_num = 10;
	ci = cl->conn_info;

	if(ioctl(btsock, HCIGETCONNLIST, (void*)cl)){
		logging_warn("could not get active rfcomm connections");
		return true; // return true to be safe here and not perform sdp scan
	}

	bool haveconn = (cl->conn_num > 0);
	logging_debug("we have " << cl->conn_num << " active hci connections");
	free(cl);
	close(btsock);

	return haveconn;
	*/
}

#endif // HAVE_BLUETOOTH_BLUETOOTH_H

}} //namespace ariba, utility
