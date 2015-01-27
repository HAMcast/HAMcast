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

#include "BaseCommunication.h"

#include "networkinfo/AddressDiscovery.h"
#include "ariba/utility/types/PeerID.h"
#include <boost/function.hpp>

#ifdef UNDERLAY_OMNET
  #include "ariba/communication/modules/transport/omnet/AribaOmnetModule.h"
  #include "ariba/communication/modules/network/omnet/OmnetNetworkProtocol.h"
  #include "ariba/utility/system/StartupWrapper.h"

  using ariba::communication::AribaOmnetModule;
  using ariba::communication::OmnetNetworkProtocol;
  using ariba::utility::StartupWrapper;
#endif

namespace ariba {
namespace communication {

using ariba::utility::PeerID;

use_logging_cpp(BaseCommunication);

/// adds an endpoint to the list
void BaseCommunication::add_endpoint( const address_v* endpoint ) {
	if (endpoint==NULL) return;
	BOOST_FOREACH( endpoint_reference& ref, remote_endpoints ) {
		if (ref.endpoint->type_id() == endpoint->type_id() && *ref.endpoint == *endpoint) {
			ref.count++;
			return;
		}
	}
	endpoint_reference ref;
	ref.endpoint = endpoint->clone();
	ref.count = 1;
	remote_endpoints.push_back(ref);
}

/// removes an endpoint from the list
void BaseCommunication::remove_endpoint( const address_v* endpoint ) {
	if (endpoint==NULL) return;
	for (vector<endpoint_reference>::iterator i = remote_endpoints.begin();
		i != remote_endpoints.end(); i++) {
		if ((*i->endpoint).type_id() == endpoint->type_id() && (*i->endpoint) == *endpoint) {
			i->count--;
			if (i->count==0) {
				logging_info("No more links to " << i->endpoint->to_string() << ": terminating transports!");
				transport->terminate(i->endpoint);
				delete i->endpoint;
				remote_endpoints.erase(i);
			}
			return;
		}
	}
}


BaseCommunication::BaseCommunication() 
	: currentSeqnum( 0 ),
	  transport( NULL ),
	  messageReceiver( NULL ),
	  started( false )
{
}


BaseCommunication::~BaseCommunication(){
}


void BaseCommunication::start() {
	logging_info( "Starting up ..." );
	currentSeqnum = 0;

	// set local peer id
	localDescriptor.getPeerId() = PeerID::random();
	logging_info( "Using PeerID: " << localDescriptor.getPeerId() );

	// creating transports
	logging_info( "Creating transports ..." );

#ifdef UNDERLAY_OMNET
	AribaOmnetModule* module = StartupWrapper::getCurrentModule();
	module->setServerPort( listenport );

	transport = module;
	network = new OmnetNetworkProtocol( module );
#else
	transport = new transport_peer( localDescriptor.getEndpoints() );
#endif

	logging_info( "Searching for local locators ..." );
	/**
	 * DONT DO THAT: if(localDescriptor.getEndpoints().to_string().length() == 0)
	 * since addresses are used to initialize transport addresses
	 */
	AddressDiscovery::discover_endpoints( localDescriptor.getEndpoints() );
	logging_info( "Done. Local endpoints = " << localDescriptor.toString() );

	transport->register_listener( this );
	transport->start();

#ifndef UNDERLAY_OMNET
	// bind to the network change detection
	networkMonitor.registerNotification( this );
#endif

	// base comm startup done
	started = true;
	logging_info( "Started up." );
}

void BaseCommunication::stop() {
	logging_info( "Stopping transports ..." );

	transport->stop();
	delete transport;
	started = false;

	logging_info( "Stopped." );
}

bool BaseCommunication::isStarted(){
	return started;
}

/// Sets the endpoints
void BaseCommunication::setEndpoints( string& _endpoints ) {
	localDescriptor.getEndpoints().assign(_endpoints);
	logging_info("Setting local end-points: "
		<< localDescriptor.getEndpoints().to_string());
}

const LinkID BaseCommunication::establishLink(
	const EndpointDescriptor& descriptor,
	const LinkID& link_id,
	const QoSParameterSet& qos,
	const SecurityParameterSet& sec) {

	// copy link id
	LinkID linkid = link_id;

	// debug
	logging_debug( "Request to establish link" );

	// create link identifier
	if (linkid.isUnspecified())	linkid = LinkID::create();

	// create link descriptor
	logging_debug( "Creating new descriptor entry with local link id=" << linkid.toString() );
	LinkDescriptor* ld = new LinkDescriptor();
	ld->localLink = linkid;
	addLink( ld );

	// send a message to request new link to remote
	logging_debug( "Send messages with request to open link to " << descriptor.toString() );
	AribaBaseMsg baseMsg( AribaBaseMsg::typeLinkRequest, linkid );
	baseMsg.getLocalDescriptor() = localDescriptor;
	baseMsg.getRemoteDescriptor().getPeerId() = descriptor.getPeerId();

	// serialize and send message
	send( &baseMsg, descriptor );

	return linkid;
}

void BaseCommunication::dropLink(const LinkID link) {

	logging_debug( "Starting to drop link " + link.toString() );

	// see if we have the link
	LinkDescriptor& ld = queryLocalLink( link );
	if( ld.isUnspecified() ) {
		logging_error( "Don't know the link you want to drop "+ link.toString() );
		return;
	}

	// tell the registered listeners
	BOOST_FOREACH( CommunicationEvents* i, eventListener ) {
		i->onLinkDown( link, ld.localLocator, ld.remoteLocator );
	}

	// create message to drop the link
	logging_debug( "Sending out link close request. for us, the link is closed now" );
	AribaBaseMsg msg( AribaBaseMsg::typeLinkClose, ld.localLink, ld.remoteLink );

	// send message to drop the link
	send( &msg, ld );

	// remove from map
	removeLink(link);
}

seqnum_t BaseCommunication::sendMessage( const LinkID lid, const Message* message) {

	logging_debug( "Sending out message to link " << lid.toString() );

	// query local link info
	LinkDescriptor& ld = queryLocalLink(lid);
	if( ld.isUnspecified() ){
		logging_error( "Don't know the link with id " << lid.toString() );
		return -1;
	}

	// link not up-> error
	if( !ld.up ) {
		logging_error("Can not send on link " << lid.toString() << ": link not up");
		return -1;
	}

	// create message
	AribaBaseMsg msg( AribaBaseMsg::typeData, ld.localLink, ld.remoteLink );

	// encapsulate the payload message
	msg.encapsulate( const_cast<Message*>(message) );

	// send message
	send( &msg, ld );

	// return sequence number
	return ++currentSeqnum;
}

const EndpointDescriptor& BaseCommunication::getEndpointDescriptor(const LinkID link) const {
	if( link.isUnspecified() ){
		return localDescriptor;
	} else {
		LinkDescriptor& linkDesc = queryLocalLink(link);
		if (linkDesc.isUnspecified()) return EndpointDescriptor::UNSPECIFIED();
		return linkDesc.remoteEndpoint;
	}
}

void BaseCommunication::registerEventListener(CommunicationEvents* _events){
	if( eventListener.find( _events ) == eventListener.end() )
		eventListener.insert( _events );
}

void BaseCommunication::unregisterEventListener(CommunicationEvents* _events){
	EventListenerSet::iterator i = eventListener.find( _events );
	if( i != eventListener.end() )
		eventListener.erase( i );
}

SystemEventType TransportEvent("Transport");
SystemEventType MessageDispatchEvent("MessageDispatchEvent", TransportEvent );

/// called when a system event is emitted by system queue
void BaseCommunication::handleSystemEvent(const SystemEvent& event) {

	// dispatch received messages
	if ( event.getType() == MessageDispatchEvent ){
		logging_debug( "Forwarding message receiver" );
		boost::function0<void>* handler = event.getData< boost::function0<void> >();
		(*handler)();
		delete handler;
	}
}

/**
 * called within the ASIO thread
 * when a message is received from underlay transport
 */ 
void BaseCommunication::receive_message(transport_connection::sptr connection,
	reboost::message_t msg) {

	logging_debug( "Dispatching message" );
	
    boost::function0<void>* handler = new boost::function0<void>(
            boost::bind(
                    &BaseCommunication::receiveMessage,
                    this,
                    connection,
                    msg)
    );
    
    SystemQueue::instance().scheduleEvent(
        SystemEvent(this, MessageDispatchEvent, handler)
    );
}

/**
 * called within the ARIBA thread (System Queue)
 * when a message is received from underlay transport
 */ 
void BaseCommunication::receiveMessage(transport_connection::sptr connection,
        reboost::message_t message)
{
    
    //// Adapt to old message system ////
    // Copy data
    size_t bytes_len = message.size();
    uint8_t* bytes = new uint8_t[bytes_len];
    message.read(bytes, 0, bytes_len);
    
    Data data(bytes, bytes_len * 8);
    
    Message legacy_message;
    legacy_message.setPayload(data);
    
    
    
	/// decapsulate message
	AribaBaseMsg* msg = legacy_message.decapsulate<AribaBaseMsg>();
	logging_debug( "Receiving message of type " << msg->getTypeString() );

	// handle message
	switch (msg->getType()) {

		// ---------------------------------------------------------------------
		// data message
		// ---------------------------------------------------------------------
		case AribaBaseMsg::typeData: {
			logging_debug( "Received data message, forwarding to overlay" );
			if( messageReceiver != NULL ) {
				messageReceiver->receiveMessage(
					msg, msg->getRemoteLink(), NodeID::UNSPECIFIED
				);
			}
			break;
		}

		// ---------------------------------------------------------------------
		// handle link request from remote
		// ---------------------------------------------------------------------
		case AribaBaseMsg::typeLinkRequest: {
			logging_debug( "Received link open request" );

			/// not the correct peer id-> skip request
			if (!msg->getRemoteDescriptor().getPeerId().isUnspecified()
				&& msg->getRemoteDescriptor().getPeerId() != localDescriptor.getPeerId()) {
				logging_info("Received link request for "
					<< msg->getRemoteDescriptor().getPeerId().toString()
					<< "but i'm "
					<< localDescriptor.getPeerId()
					<< ": Ignoring!");
				break;
			}

			/// only answer the first request
			if (!queryRemoteLink(msg->getLocalLink()).isUnspecified()) {
				logging_debug("Link request already received. Ignore!");
				break;
			}

			/// create link ids
			LinkID localLink  = LinkID::create();
			LinkID remoteLink = msg->getLocalLink();
			logging_debug(
			        "local=" << connection->getLocalEndpoint()->to_string()
				<< " remote=" << connection->getRemoteEndpoint()->to_string()
			);

			// check if link creation is allowed by ALL listeners
			bool allowlink = true;
			BOOST_FOREACH( CommunicationEvents* i, eventListener ){
				allowlink &= i->onLinkRequest( localLink,
				        connection->getLocalEndpoint(),
				        connection->getRemoteEndpoint());
			}

			// not allowed-> warn
			if( !allowlink ){
				logging_warn( "Overlay denied creation of link" );
				delete msg;
				return;
			}

			// create descriptor
			LinkDescriptor* ld = new LinkDescriptor();
			ld->localLink = localLink;
			ld->remoteLink = remoteLink;
			ld->localLocator = connection->getLocalEndpoint()->clone();
			ld->remoteLocator = connection->getRemoteEndpoint()->clone();
			ld->connection = connection;
			ld->remoteEndpoint = msg->getLocalDescriptor();
			add_endpoint(ld->remoteLocator);

			// add layer 1-3 addresses
			ld->remoteEndpoint.getEndpoints().add(
				ld->remoteLocator, endpoint_set::Layer1_3 | endpoint_set::NoLoopback);
			localDescriptor.getEndpoints().add(
				connection->getLocalEndpoint(),
				endpoint_set::Layer1_3 | endpoint_set::NoLoopback);

			// link is now up-> add it
			ld->up = true;
			addLink(ld);

			// link is up!
			logging_debug( "Link (initiated from remote) is up with "
				<< "local(id=" << ld->localLink.toString() << ","
				<< "locator=" << ld->localLocator->to_string() << ") "
				<< "remote(id=" << ld->remoteLink.toString() << ", "
				<< "locator=" << ld->remoteLocator->to_string() << ")"
			);

			// sending link request reply
			logging_debug( "Sending link request reply with ids "
				<< "local=" << localLink.toString() << ", "
				<< "remote=" << remoteLink.toString() );
			AribaBaseMsg reply( AribaBaseMsg::typeLinkReply, localLink, remoteLink );
			reply.getLocalDescriptor() = localDescriptor;
			reply.getRemoteDescriptor() = ld->remoteEndpoint;

			send( &reply, *ld );

			// inform listeners about new open link
			BOOST_FOREACH( CommunicationEvents* i, eventListener ) {
				i->onLinkUp( localLink, ld->localLocator, ld->remoteLocator);
			}

			// done
			break;
		}

		// ---------------------------------------------------------------------
		// handle link request reply
		// ---------------------------------------------------------------------
		case AribaBaseMsg::typeLinkReply: {
			logging_debug( "Received link open reply for a link we initiated" );

			// this is a reply to a link open request, so we have already
			// a link mapping and can now set the remote link to valid
			LinkDescriptor& ld = queryLocalLink( msg->getRemoteLink() );

			// no link found-> warn!
			if (ld.isUnspecified()) {
				logging_warn("Failed to find local link " << msg->getRemoteLink().toString());
				delete msg;
				return;
			}

			// store the connection
			ld.connection = connection;
			
			// set remote locator and link id
			ld.remoteLink = msg->getLocalLink();
			ld.remoteLocator = connection->getRemoteEndpoint()->clone();
			ld.remoteEndpoint.getEndpoints().add(
							msg->getLocalDescriptor().getEndpoints(),
							endpoint_set::Layer1_4
						);

			localDescriptor.getEndpoints().add(
				msg->getRemoteDescriptor().getEndpoints(),
				endpoint_set::Layer1_3
			);
			ld.up = true;
			add_endpoint(ld.remoteLocator);

			logging_debug( "Link is now up with local id "
				<< ld.localLink.toString() << " and remote id "
				<< ld.remoteLink.toString() );


			// inform lisneters about link up event
			BOOST_FOREACH( CommunicationEvents* i, eventListener ){
				i->onLinkUp( ld.localLink, ld.localLocator, ld.remoteLocator );
			}

			// done
			break;
		}

		// ---------------------------------------------------------------------
		// handle link close requests
		// ---------------------------------------------------------------------
		case AribaBaseMsg::typeLinkClose: {
			// get remote link
			const LinkID& localLink = msg->getRemoteLink();
			logging_debug( "Received link close request for link " << localLink.toString() );

			// searching for link, not found-> warn
			LinkDescriptor& linkDesc = queryLocalLink( localLink );
			if (linkDesc.isUnspecified()) {
				logging_warn("Failed to find local link " << localLink.toString());
				delete msg;
				return;
			}

			// inform listeners
			BOOST_FOREACH( CommunicationEvents* i, eventListener ){
				i->onLinkDown( linkDesc.localLink,
						linkDesc.localLocator, linkDesc.remoteLocator );
			}

			// remove the link descriptor
			removeLink( localLink );

			// done
			break;
		}

		// ---------------------------------------------------------------------
		// handle link locator changes
		// ---------------------------------------------------------------------
		case AribaBaseMsg::typeLinkUpdate: {
			const LinkID& localLink = msg->getRemoteLink();
			logging_debug( "Received link update for link "
				<< localLink.toString() );

			// find the link description
			LinkDescriptor& linkDesc = queryLocalLink( localLink );
			if (linkDesc.isUnspecified()) {
				logging_warn("Failed to update local link "
					<< localLink.toString());
				delete msg;
				return;
			}

			// update the remote locator
			const address_v* oldremote = linkDesc.remoteLocator;
			linkDesc.remoteLocator = connection->getRemoteEndpoint()->clone();

			// inform the listeners (local link has _not_ changed!)
			BOOST_FOREACH( CommunicationEvents* i, eventListener ){
				i->onLinkChanged(
					linkDesc.localLink,	// linkid
					linkDesc.localLocator,	// old local
					linkDesc.localLocator,	// new local
					oldremote,		// old remote
					linkDesc.remoteLocator	// new remote
				);
			}

			// done
			break;
		}
	}

	delete msg;
}

/// add a newly allocated link to the set of links
void BaseCommunication::addLink( LinkDescriptor* link ) {
	linkSet.push_back( link );
}

/// remove a link from set
void BaseCommunication::removeLink( const LinkID& localLink ) {
	for(LinkSet::iterator i=linkSet.begin(); i != linkSet.end(); i++){
		if( (*i)->localLink != localLink) continue;
		remove_endpoint((*i)->remoteLocator);
		delete *i;
		linkSet.erase( i );
		break;
	}
}

/// query a descriptor by local link id
BaseCommunication::LinkDescriptor& BaseCommunication::queryLocalLink( const LinkID& link ) const {
	for (size_t i=0; i<linkSet.size();i++)
		if (linkSet[i]->localLink == link) return (LinkDescriptor&)*linkSet[i];

	return LinkDescriptor::UNSPECIFIED();
}

/// query a descriptor by remote link id
BaseCommunication::LinkDescriptor& BaseCommunication::queryRemoteLink( const LinkID& link ) const {
	for (size_t i=0; i<linkSet.size();i++)
		if (linkSet[i]->remoteLink == link) return (LinkDescriptor&)*linkSet[i];

	return LinkDescriptor::UNSPECIFIED();
}

LinkIDs BaseCommunication::getLocalLinks( const address_v* addr ) const {
	LinkIDs ids;
	for (size_t i=0; i<linkSet.size(); i++){
		if( addr == NULL ){
			ids.push_back( linkSet[i]->localLink );
		} else {
			if ( *linkSet[i]->remoteLocator == *addr )
				ids.push_back( linkSet[i]->localLink );
		}
	}
	return ids;
}

void BaseCommunication::onNetworkChange(const NetworkChangeInterface::NetworkChangeInfo& info){

#ifdef UNDERLAY_OMNET

	// we have no mobility support for simulations
	return

#endif // UNDERLAY_OMNET

/*- disabled!

	// we only care about address changes, not about interface changes
	// as address changes are triggered by interface changes, we are safe here
	if( info.type != NetworkChangeInterface::EventTypeAddressNew &&
		info.type != NetworkChangeInterface::EventTypeAddressDelete ) return;

	logging_info( "base communication is handling network address changes" );

	// get all now available addresses
	NetworkInformation networkInformation;
	AddressInformation addressInformation;

	NetworkInterfaceList interfaces = networkInformation.getInterfaces();
	AddressList addresses;

	for( NetworkInterfaceList::iterator i = interfaces.begin(); i != interfaces.end(); i++ ){
		AddressList newaddr = addressInformation.getAddresses(*i);
		addresses.insert( addresses.end(), newaddr.begin(), newaddr.end() );
	}

	//
	// get current locators for the local endpoint
	// TODO: this code is dublicate of the ctor code!!! cleanup!
	//

	NetworkProtocol::NetworkLocatorSet locators = network->getAddresses();
	NetworkProtocol::NetworkLocatorSet::iterator i = locators.begin();
	NetworkProtocol::NetworkLocatorSet::iterator iend = locators.end();

	//
	// remember the old local endpoint, in case it changes
	//

	EndpointDescriptor oldLocalDescriptor( localDescriptor );

	//
	// look for local locators that we can use in communication
	//
	// choose the first locator that is not localhost
	//

	bool foundLocator = false;
	bool changedLocator = false;

	for( ; i != iend; i++){
		logging_debug( "local locator found " << (*i)->toString() );
		IPv4Locator* ipv4locator = dynamic_cast<IPv4Locator*>(*i);

		if( *ipv4locator != IPv4Locator::LOCALHOST &&
		    *ipv4locator != IPv4Locator::ANY       &&
		    *ipv4locator != IPv4Locator::BROADCAST  ){

			ipv4locator->setPort( listenport );
			changedLocator = *localDescriptor.locator != *ipv4locator;
			localDescriptor.locator = ipv4locator;
			logging_info( "binding to addr = " << ipv4locator->toString() );
			foundLocator = true;
			break;
		}
	} // for( ; i != iend; i++)

	//
	// if we found no locator, bind to localhost
	//

	if( !foundLocator ){
		changedLocator = *localDescriptor.locator != IPv4Locator::LOCALHOST;
		localDescriptor.locator = new IPv4Locator( IPv4Locator::LOCALHOST );
		((IPv4Locator*)(localDescriptor.locator))->setPort( listenport );
		logging_info( "found no good local lcoator, binding to addr = " <<
						localDescriptor.locator->toString() );
	}

	//
	// if we have connections that have no more longer endpoints
	// close these. they will be automatically built up again.
	// also update the local locator in the linkset mapping
	//

	if( changedLocator ){

		logging_debug( "local endp locator has changed to " << localDescriptor.toString() <<
				", resettings connections that end at old locator " <<
					oldLocalDescriptor.toString());

		LinkSet::iterator i = linkSet.begin();
		LinkSet::iterator iend = linkSet.end();

		for( ; i != iend; i++ ){

			logging_debug( "checking connection for locator change: " <<
					" local " << (*i).localLocator->toString() <<
					" old " << oldLocalDescriptor.locator->toString() );

			if( *((*i).localLocator) == *(oldLocalDescriptor.locator) ){

				logging_debug("terminating connection to " << (*i).remoteLocator->toString() );
				transport->terminate( oldLocalDescriptor.locator, (*i).remoteLocator );

				(*i).localLocator = localDescriptor.locator;
			}
		} // for( ; i != iend; i++ )

		// wait 500ms to give the sockets time to shut down
		usleep( 500000 );

	} else {

		logging_debug( "locator has not changed, not resetting connections" );

	}

	//
	// handle the connections that have no longer any
	// valid locator. send update messages with the new
	// locator,  so the remote node updates its locator/link mapping
	//

	LinkSet::iterator iAffected = linkSet.begin();
	LinkSet::iterator endAffected = linkSet.end();

	for( ; iAffected != endAffected; iAffected++ ){
		LinkDescriptor descr = *iAffected;
		logging_debug( "sending out link locator update to " << descr.remoteLocator->toString() );

		AribaBaseMsg updateMsg( 	descr.remoteLocator,
						AribaBaseMsg::LINK_STATE_UPDATE,
						descr.localLink, descr.remoteLink );

		transport->sendMessage( &updateMsg );
	}
*/
}

/// sends a message to all end-points in the end-point descriptor
void BaseCommunication::send(Message* legacy_message, const EndpointDescriptor& endpoint) {
	Data data = data_serialize(legacy_message, DEFAULT_V);
	
	//// Adapt to new message system ////
	// transfer data buffer ownership to the shared_buffer
    reboost::shared_buffer_t buf(data.getBuffer(), data.getLength() / 8);
	
	reboost::message_t message;
	message.push_back(buf);
	
	transport->send(endpoint.getEndpoints(), message);
}

/// sends a message to the remote locator inside the link descriptor
void BaseCommunication::send(Message* legacy_message, const LinkDescriptor& desc) {
	if (desc.remoteLocator==NULL) return;
	
	Data data = data_serialize(legacy_message, DEFAULT_V);
    
    //// Adapt to new message system ////
    // transfer data buffer ownership to the shared_buffer
    reboost::shared_buffer_t buf(data.getBuffer(), data.getLength() / 8);
    
    reboost::message_t message;
    message.push_back(buf);
    
	desc.connection->send(message);
}

}} // namespace ariba, communication
