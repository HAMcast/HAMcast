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

#ifndef __PERIODIC_BROADCAST_H
#define __PERIODIC_BROADCAST_H

#include "ariba/config.h"

#include <map>
#include <string>
#include <ctime>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "ariba/utility/bootstrap/modules/BootstrapModule.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/system/Timer.h"
#include "PeriodicBroadcastMessage.h"

using std::map;
using std::string;
using std::cout;
using boost::asio::ip::udp;

namespace ariba {
namespace utility {

class PeriodicBroadcast : public BootstrapModule, public Timer {
	use_logging_h(PeriodicBroadcast);
public:
	PeriodicBroadcast(BootstrapInformationCallback* _callback, string info);
	virtual ~PeriodicBroadcast();

	virtual void start();
	virtual void stop();

	virtual string getName();
	virtual string getInformation();
	virtual bool isFunctional();
	virtual void publishService(string name, string info1, string info2, string info3);
	virtual void revokeService(string name);

protected:
	virtual void eventFunction();

private:
	void sendLocalServices();
	void updateRemoteServices();

	static const long timerinterval; // used to send out updates on our services and check for new services
	static const long servicetimeout; // timeout after that a service is dead when we did not receive updates
	static const unsigned int serverport_v4;
	static const unsigned int serverport_v6;

	class Service {
	private:
		string name;
		string info1;
		string info2;
		string info3;
		time_t lastseen;

	public:
		Service()
			: name(""), info1(""), info2(""), info3(""), lastseen(0){
		}

		Service(const string& _name, const string& _info1,
				const string& _info2, const string& _info3, const time_t& _lastseen = 0){
			name.assign (_name);
			info1.assign(_info1);
			info2.assign(_info2);
			info3.assign(_info3);
			lastseen = _lastseen;
		}

		Service(const Service& rh){
			name.assign (rh.name);
			info1.assign(rh.info1);
			info2.assign(rh.info2);
			info3.assign(rh.info3);
			lastseen = rh.lastseen;
		}

		string getName() const {
			return name;
		}

		string getInfo1() const {
			return info1;
		}

		string getInfo2() const {
			return info2;
		}

		string getInfo3() const {
			return info3;
		}

		time_t getLastseen() const {
			return lastseen;
		}

		void setName(string _name){
			name.assign(_name);
		}

		void setInfo1(string _info1){
			info1.assign(_info1);
		}

		void setInfo2(string _info2){
			info2.assign(_info2);
		}

		void setInfo3(string _info3){
			info3.assign(_info3);
		}

		void setLastseen(time_t _lastseen){
			lastseen = _lastseen;
		}

		Service& operator=(const Service& rh){
			this->name.assign( rh.getName() );
			this->info1.assign( rh.getInfo1() );
			this->info2.assign( rh.getInfo2() );
			this->info3.assign( rh.getInfo3() );
			this->lastseen = rh.lastseen;
			return *this;
		}
	};

	typedef map<string,Service> ServiceList;

	ServiceList localServices;
	boost::mutex localServicesMutex;

	ServiceList remoteServices;
	boost::mutex remoteServicesMutex;

	ServiceList newRemoteServices;
	boost::mutex newRemoteServicesMutex;

	boost::asio::io_service io_service;
	boost::thread* io_service_thread;
	static void threadFunc(PeriodicBroadcast* obj);

	class udp_server {
	private:
		udp::socket socket_v4;
		udp::socket socket_v6;
		udp::endpoint remote_endpoint_;
		boost::array<char, 1500> recv_buffer_4;
		boost::array<char, 1500> recv_buffer_6;
		ServiceList* services;
		boost::mutex* servicesmutex;

	public:
		udp_server(boost::asio::io_service& io_service, ServiceList* _services, boost::mutex* _servicesmutex)
			: socket_v4(io_service), socket_v6(io_service),
			  services(_services), servicesmutex(_servicesmutex) {

			if( open4() ) start_receive_4();
			if( open6() ) start_receive_6();
		}

		bool open4(){
			boost::system::error_code err;

			boost::asio::ip::udp::endpoint listen_endpoint_v4(
					boost::asio::ip::address_v4::any(),
					PeriodicBroadcast::serverport_v4);

			err = socket_v4.open( listen_endpoint_v4.protocol(), err );
			if(err){
				logging_warn("failed opening ipv4 socket");
				return false;
			}

			err = socket_v4.set_option( boost::asio::ip::udp::socket::reuse_address(true), err );
			if(err){
				logging_warn("failed setting reuse address option on ipv4 socket");
				return false;
			}

			err = socket_v4.set_option( boost::asio::socket_base::broadcast(true), err );
			if(err){
				logging_warn("failed setting broadcast option on ipv4 socket");
				return false;
			}

			err = socket_v4.bind( listen_endpoint_v4, err );
			if(err){
				logging_warn("failed binding ipv4 socket");
				return false;
			}

			return true;
		}

		bool open6(){
			boost::system::error_code err;

			boost::asio::ip::udp::endpoint listen_endpoint_v6(
					boost::asio::ip::address_v6::any(),
					PeriodicBroadcast::serverport_v6);

			err = socket_v6.open( listen_endpoint_v6.protocol(), err );
			if(err){
				logging_warn("failed opening ipv6 socket");
				return false;
			}

			err = socket_v6.set_option( boost::asio::ip::udp::socket::reuse_address(true), err );
			if(err){
				logging_warn("failed setting reuse address option on ipv6 socket");
				return false;
			}

			err = socket_v6.set_option( boost::asio::socket_base::broadcast(true), err );
			if(err){
				logging_warn("failed setting broadcast option on ipv6 socket");
				return false;
			}

			err = socket_v6.bind( listen_endpoint_v6, err );
			if(err){
				logging_warn("failed binding ipv6 socket");
				return false;
			}

			return true;
		}

		void sendservice(Service service){

			PeriodicBroadcastMessage msg;
			if(service.getName().empty()) return;

			msg.setName( service.getName() );
			msg.setInfo1( service.getInfo1() );
			msg.setInfo2( service.getInfo2() );
			msg.setInfo3( service.getInfo3() );

			Data data = data_serialize( msg, DEFAULT_V );
			uint8_t* pnt = data.getBuffer();
			size_t len = data.getLength() / 8;

			boost::system::error_code err;

			{
				udp::endpoint endp(udp::v4(), PeriodicBroadcast::serverport_v4);
				endp.address( boost::asio::ip::address_v4::broadcast() );
				socket_v4.send_to( boost::asio::buffer(pnt, len), endp, 0, err );
				if(err) logging_warn("failed sending message through ipv4 socket");
			}
			{
				udp::endpoint endp(udp::v6(), PeriodicBroadcast::serverport_v6);
				endp.address( boost::asio::ip::address_v6::from_string("ff02::1") );
				socket_v6.send_to( boost::asio::buffer(pnt, len), endp, 0, err );
				if(err) logging_warn("failed sending message through ipv6 socket");
			}
		}

	private:
		void start_receive_4(){
			socket_v4.async_receive_from(
					boost::asio::buffer(recv_buffer_4), remote_endpoint_,
					boost::bind(&udp_server::handle_receive_4, this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
		}

		void start_receive_6(){
			socket_v6.async_receive_from(
					boost::asio::buffer(recv_buffer_6), remote_endpoint_,
					boost::bind(&udp_server::handle_receive_6, this,
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
		}

		void handle_receive_4(const boost::system::error_code& error,
				std::size_t bytes_transferred){

			if (!error || error == boost::asio::error::message_size)
				handle_info(recv_buffer_4, bytes_transferred);
			else
				logging_warn("failed receiving broadcast data: " << error.message());

			start_receive_4();
		}

		void handle_receive_6(const boost::system::error_code& error,
				std::size_t bytes_transferred){

			if (!error || error == boost::asio::error::message_size)
				handle_info(recv_buffer_6, bytes_transferred);
			else
				logging_warn("failed receiving broadcast data: " << error.message());

			start_receive_6();
		}

		void handle_info(boost::array<char, 1500>& buffer, std::size_t length){

			try {

				PeriodicBroadcastMessage msg;

				Data data( (uint8_t*)buffer.data(), length*8 );
				data_deserialize( msg, data );

				{ // insert new found service
					boost::mutex::scoped_lock lock( *servicesmutex );
					if(msg.getName().empty()) return;

					ServiceList::iterator it = services->find( msg.getName() );
					if( it != services->end() ){
						it->second.setLastseen( time(NULL) );
					} else {
						Service s( msg.getName(), msg.getInfo1(), msg.getInfo2(), msg.getInfo3(), time(NULL));
						services->insert( std::make_pair(msg.getName(), s) );
					}
				}

			}catch(...){
				/* ignore error */
			}
		}

		void handle_send(boost::shared_ptr<std::string> /*message*/,
				const boost::system::error_code& error,
				std::size_t /*bytes_transferred*/){

			if(error)
				logging_warn("failed sending out message");
		}
	};

	udp_server server;
};

}} //namespace ariba, utility

#endif // __BLUETOOTH_SDP_H
