// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Karlsruhe Institute of Technology (KIT)
//
// Institute of Telematics
// Karlsruhe Institute of Technology (KIT)
// Zirkel 2, 76128 Karlsruhe
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
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OF TELEMATICS OR
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
/**
 * @file MCPO.cpp
 * @author Christian Huebsch
 */

#include "MCPO.h"

namespace ariba {
namespace services {
namespace mcpo {

use_logging_cpp(MCPO);

/** ***************************************************************************
 * MCPO Constructor
 * @param receiver 		Callback interface from application
 * @param _ariba 		The Ariba Context
 * @param _node 		The Node Context
 * @param _clioNode		The CLIO Context - optional
 * @param useVisuals 	Indicate if visualizations should be used
 *****************************************************************************/
MCPO::MCPO( ReceiverInterface* receiver, ServiceID sID, AribaModule *_ariba, Node *_node, /*CLIONode *_clioNode,*/ bool _useVisuals )
	: useVisuals ( _useVisuals),
	  receiver ( receiver ),
	  ariba ( _ariba ),
	  node ( _node ),
	  /*clioNode ( _clioNode ),*/
	  heartbeatTimer ( NULL ),
	  refineTimer ( NULL ),
	  queryTimer ( NULL ),
	  backoffTimer ( NULL ),
	  rpcollisionTimer ( NULL ),
	  structureTimer ( NULL ),
      bootstrapTimer ( NULL ),
      serviceID(sID)
{
	logging_info("creating mcpo");

	// Get own NodeID
	myNodeID = node->getNodeId();

	// Assign service id of the mcpo service
	if(sID==NULL)sID=ariba::utility::ServiceID(666);

	// Bind service to Ariba
	node->bind( this, serviceID); /*CommunicationListener*/

//for ariba 0.8.1
    dht=new ariba_service::dht::Dht(sID, node);
	isReady = false;

	// Set backoff timer, giving ariba time to manage stuff
	backoffTimer = new BackoffTimer( this );
	//backoffTimer->setInterval( 6000, true );
    backoffTimer->setInterval( 5, true );
    backoffTimer->start();

	ariba::utility::Helper::getElapsedMillis();

	// As soon as backoff timer fires, initializeMCPO is called

} // MCPO

MCPO::MCPO( ReceiverInterface* receiver, AribaModule *_ariba, Node *_node, /*CLIONode *_clioNode,*/ bool _useVisuals )
	: useVisuals ( _useVisuals),
	  receiver ( receiver ),
	  ariba ( _ariba ),
	  node ( _node ),
      serviceID(666),
	  /*clioNode ( _clioNode ),*/
	  heartbeatTimer ( NULL ),
	  refineTimer ( NULL ),
	  queryTimer ( NULL ),
	  backoffTimer ( NULL ),
	  rpcollisionTimer ( NULL ),
	  structureTimer ( NULL ),
      bootstrapTimer ( NULL )

{
	logging_info("creating mcpo");

	// Get own NodeID
	myNodeID = node->getNodeId();

    //for ariba 0.8.1
    dht=new ariba_service::dht::Dht(666,node);


	// Bind service to Ariba
	node->bind( this, serviceID); /*CommunicationListener*/

	isReady = false;

	// Set backoff timer, giving ariba time to manage stuff
	backoffTimer = new BackoffTimer( this );
   backoffTimer->setInterval( 0, true );
    backoffTimer->start();

	ariba::utility::Helper::getElapsedMillis();

	// As soon as backoff timer fires, initializeMCPO is called

} // MCPO



/** ***************************************************************************
 * MCPO Destructor
 *****************************************************************************/
MCPO::~MCPO()
{
	if (useVisuals)
		visual.visShutdown(visualIdMcpo, myNodeID, "");

	// Cancel and delete all timers
	if(backoffTimer != NULL){
		backoffTimer->stop();
		delete backoffTimer;
		backoffTimer = NULL;
	}

	if(heartbeatTimer != NULL){
		heartbeatTimer->stop();
		delete heartbeatTimer;
		heartbeatTimer = NULL;
	}

	if(refineTimer != NULL){
		refineTimer->stop();
		delete refineTimer;
		refineTimer = NULL;
	}

	if(queryTimer != NULL){
		queryTimer->stop();
		delete queryTimer;
		queryTimer = NULL;
	}

	if(structureTimer != NULL){
		structureTimer->stop();
		delete structureTimer;
		structureTimer = NULL;
	}

	if(bootstrapTimer != NULL){
		bootstrapTimer->stop();
		delete bootstrapTimer;
		bootstrapTimer = NULL;
	}

	// Delete all structures
	std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.begin();

	for (; it != peerInfos.end(); it++)
		delete it->second;

} // ~MCPOs

/** ***************************************************************************
 * Initializing local MCPO instance
 *
 * In this method, timers and variables get initialized
 *****************************************************************************/
void MCPO::initializeMCPO()
{

	logging_info("initializing mcpo");

	/* Add own node to peerInfos */
	MCPOPeerInfo* pi = new MCPOPeerInfo(this);
	pi->set_distance(0);
	peerInfos.insert(std::make_pair(myNodeID, pi));

	/* Set evaluation layer to not specified */
	evalLayer = -1;
    joinLayer = -1;

	for (int i=0; i<10; i++) {

		visLinks[i].clear();

	}

	first_leader = NodeID::UNSPECIFIED;
	second_leader = NodeID::UNSPECIFIED;

	// Create structure for cluster information maintenance
	clusters = new MCPOCluster[10];

	/* Initially clear all clusters */
	for (int i=0; i<10; i++) {

		clusters[i].clear();

	}

	// Initialize Heartbeat timer for periodic heartbeats
	heartbeatTimer = new HeartbeatTimer( this );
	heartbeatTimer->setInterval( 2000 );

	// Initialize Refine timer for periodic protocol refinement
	refineTimer = new RefineTimer( this );
	refineTimer->setInterval( 3300 );

	// Timer for layer query timeouts
	queryTimer = new QueryTimer( this );
    queryTimer->setInterval(2000, true);
  //  queryTimer->setInterval(0, true);

	// Timer for detection of structure partition
	structureTimer = new StructureTimer( this );
	structureTimer->setInterval(5000);

	// Initialize Bootstrap timer for One-Shot bootstrapping
	bootstrapTimer = new BootstrapTimer( node, this, &clusters[0] );
   bootstrapTimer->setInterval( 2000, true );
    //bootstrapTimer->setInterval( 0, true );
	rpcollisionTimer = new RPCollisionTimer( this );
	rpcollisionTimer->setInterval( 10000 );

	// cluster parameter k
    k = 2;

	//use DHT?
	useDHT = false;
	//useDHT = true;

	//use CLIO?
	useCLIO = false;
	#ifdef HAVE_CLIO_CLIO_H
		useCLIO = true;
	#endif

	peerTimeoutInterval = 10;

	SC_PROC_DISTANCE = 30;
	SC_MIN_OFFSET = 100;

	if (useVisuals)
		visual.visCreate(visualIdMcpo, myNodeID, "", "");

	// Switch to INIT state
	changeState( INIT );

} // initializeMCPO


/** ***************************************************************************
 *  Change MCPO protocol state
 *
 * MCPO employs three protocol states which reflect initialization,
 * bootstrapping and normal working state
 * Changing current state is accomplished here
 * @param state Protocol state to change to
 *****************************************************************************/
void MCPO::changeState( int toState )
{
	switch (toState) {

    	case INIT:

    		logging_info("changing state to INIT");

    		// First, check if there is already a RP
    		sendRPSearchMessage();

       	 	break;

    	case BOOTSTRAP:

    		logging_info("changing state to BOOTSTRAP");

   			if ( Rendevouz == myNodeID ) {

    			logging_info("I am current Rendevouz Point");
                std::cout << "current RP for Group: " << this->serviceID.toString()<< std::endl;

    			/* join cluster layer 0 as first node */
    			clusters[0].add( myNodeID );
    			clusters[0].setLeader( myNodeID );

    			changeState(READY);

    			return;

    		} else {

    			/* initiate NICE structure joining */
    			BasicJoinLayer(-1);

    			structureTimer->start();

    		}

    		break;

    	case READY:

        	logging_info("changing state to READY");
        	logging_info("setting Refine Interval to: 3.3");

        	// Start periodic timers
        	refineTimer->start();
        	heartbeatTimer->start();

        	/* indicate to application that service is ready */
        	receiver->serviceIsReady();

        	break;

    }


} // changeState


/*****************************************************************************
 * Search for Rendezvouz Point via Ariba-Broadcast or DHT
 *****************************************************************************/
void MCPO::sendRPSearchMessage()
{

	if (!useDHT) {

		logging_info("sending RP search message via BO Broadcast");

		MCPOMsg mcpomsg( MCPOMsg::MCPO_LOOKUP_RP, myNodeID );

		node->sendBroadcastMessage( &mcpomsg, serviceID );
		bootstrapTimer->start();


    } else {

		//DHT usage
		logging_info("Querying DHT for MCPO_RP key");

		bootstrapTimer->start();
		//Search for existing RP
		Identifier key = Identifier::sha1("MCPO_RP");
       // node->get(data_serialize(key), serviceID);

        //for ariba 0.8.1
           dht->get(key.toString());
	}

	logging_info("initiating timer with delay: 2000 ms");

	//Setting MCPO to ready
	isReady = true;

} // sendRPSearchMessage


/** ***************************************************************************
 * Sets Rendevouz Point
 * @param id The RPs NodeID
 *****************************************************************************/
void MCPO::setRendevouz( NodeID id )
{
	logging_info("setting RP to " + id.toString());
	Rendevouz = id;

	if (useDHT) {
		//If I am RP, put to DHT
		if (id == node->getNodeId()) {

			Identifier key = Identifier::sha1("MCPO_RP");
     //     node->put(data_serialize(key), data_serialize(&id), 3600, true);

          //for ariba 0.8.1
        std::string key1=key.toString();
            std::string id1=id.toString();
           dht->put(key1,id1,3600);

		}
	}

} // setRendevouz

/** ***************************************************************************
 * Handle RP Search Message
 * @param srcNode The sender's NodeID
 *****************************************************************************/
void MCPO::handleRPSearchMessage( NodeID srcNode )
{

	logging_info("handling RP search inquiry by " + srcNode.toString());

	// If I am Rendevouz Point, reply. Else: Ignore
	if ( Rendevouz == myNodeID )
		sendRPReplyMessage( srcNode );

} // handleRPSeachMessage


/** ***************************************************************************
 * Send RP Reply Message
 * @param dest NodeID to reply to
 *****************************************************************************/
void MCPO::sendRPReplyMessage( NodeID dest )
{

	logging_info("sending RP reply to " + dest.toString());

	MCPOMsg mcpomsg( MCPOMsg::MCPO_LOOKUP_RP_REPLY, myNodeID );

	node->sendMessage( &mcpomsg, dest, serviceID );

} // sendRPReplyMessage


/** ***************************************************************************
 * Handle RP Reply Message
 * @param rendevouz The Rendevouz Point's NodeID
 *****************************************************************************/
void MCPO::handleRPReplyMessage( NodeID rendevouz )
{

	logging_info("handling RP reply from " + rendevouz.toString());

	if ((Rendevouz == myNodeID) && (rendevouz > myNodeID)) {

		// Foreign RP may stay, merge with his cluster
		logging_info("Foreign RP may stay, merge with his cluster");

		// send merge request
		ClusterMergeRequest(rendevouz, 0 );

		clusters[0].setLeader(rendevouz);

		setRendevouz(rendevouz);

	} else if ((Rendevouz == myNodeID) && (rendevouz < myNodeID)) {

		//setRendevouz(rendevouz);
		//logging_info("Set Rendevouz to: " <<  rendevouz);
		//structureTimer->start();

	} else {

		logging_info("Set Rendevouz to: " <<  rendevouz);

		// For only the RP answers, assume the Src of msg to be RP
		setRendevouz( rendevouz );

		bootstrapTimer->stop();

		structureTimer->start();

		//We now know the RP, switch to Bootstrapping phase
		changeState( BOOTSTRAP );

	}


} // handleRPReplyMessage


/** ***************************************************************************
 * BasicJoinLayer
 * @param layer The layer intended to join
 *****************************************************************************/
void MCPO::BasicJoinLayer(short layer)
{

	logging_info("BasicJoinLayer: " << layer);

	if (Rendevouz.isUnspecified()) {

		setRendevouz(myNodeID);
		return;

	}


	Query(Rendevouz, layer);

	if (layer > -1)
		targetLayer = layer;
	else
		targetLayer = 0;

	// Temporary peer with RP for faster data reception
	MCPOMsg mcpomsg( MCPOMsg::MCPO_PEER_TEMPORARY, myNodeID );

	node->sendMessage( &mcpomsg, Rendevouz, serviceID );

	isTempPeered = true;

} // BasicJoinLayer


/** ***************************************************************************
 * Query Layer Information for iterative structure joining
 * @param destination	Node to query
 * @param layer			Layer to query
 *****************************************************************************/
void MCPO::Query(const NodeID& destination, short layer)
{

	logging_info("sending membership query for layer "
		<< layer << " to " 	<<  destination.toString());

	MCPOMsg mcpomsg( MCPOMsg::MCPO_QUERY, myNodeID, layer );

	query_start = ariba::utility::Helper::getElapsedMillis();
	tempResolver = destination;
	logging_info("set tempResolver to: " << destination);

	queryTimer->stop();
	queryTimer = new QueryTimer( this );
	queryTimer->setInterval( 2000 );
	queryTimer->start();

	joinLayer = layer;

	node->sendMessage( &mcpomsg, destination, serviceID );

} // Query


/** ***************************************************************************
 * Handle Membership Query Message
 * @param msg	Query message to handle
 *****************************************************************************/
void MCPO::handleMembershipQuery( MCPOMsg* msg )
{

	NodeID srcNode = msg->getSrcNode();
	int layer = msg->getLayer();

	logging_info("handling membership query for layer "
		<< layer << " from " <<  srcNode.toString());

	logging_info("getHighestLeaderLayer() " << getHighestLeaderLayer());

	if (layer > getHighestLeaderLayer()) {

		return;

	}

	if (layer < 0) {

		if (Rendevouz == myNodeID) {

			/* If layer is < 0, response with highest layer I am leader of */
			layer = getHighestLeaderLayer();

		} else {

			logging_info("forwarding membership query for layer "
					<< layer << " to real RP: " 	<<  Rendevouz.toString());

			MCPOMsg mcpomsg( MCPOMsg::MCPO_QUERY, srcNode, layer );

			node->sendMessage( &mcpomsg, Rendevouz, serviceID );

			return;

		}

	}

	logging_info("sending membership reply for layer "
		<< layer << " to "
		<<  srcNode.toString());

	MCPOMsg mcpomsg( MCPOMsg::MCPO_QUERY_RESPONSE, myNodeID, layer );
	MCPOMemberMsg membermsg;

	for (int i = 0; i < clusters[layer].getSize(); i++) {

		const NodeID node = clusters[layer].get(i);

		if (node != myNodeID) {
			logging_info("Adding: " << node.toString());
			membermsg.insert( new NodeID( node ) );
		}

	}

	mcpomsg.encapsulate( &membermsg );
	node->sendMessage( &mcpomsg, srcNode, serviceID );

} // handleMembershipQuery


/** ***************************************************************************
 * getHighestLeaderLayer
 * @return The highest layer the node is leader in
 *****************************************************************************/
short MCPO::getHighestLeaderLayer()
{

	short highest = -1;

	for (short i=0; i<10; i++) {

		if (!clusters[i].getLeader().isUnspecified()) {

			if (clusters[i].getLeader() == myNodeID)
					highest = i;
		}

	}

	return highest;

} // getHighestLeaderLayer


/** ***************************************************************************
 * getHighestLayer
 * @return The highest layer the node resides in
 *****************************************************************************/
short MCPO::getHighestLayer()
{

	short highest = -1;
	for (short i=0; i<10; i++) {

		if (clusters[i].contains(myNodeID))

			highest = i;

	}

	return highest;

} // getHighestLayer


/** ***************************************************************************
 * Handle Membership Reply Message
 * @param msg RP Reply Message from Cluster Leader
 *****************************************************************************/
void MCPO::handleMembershipReply( MCPOMsg* msg )
{
	if (queryTimer->isRunning()) {
	  queryTimer->stop();			       
	}

	NodeID srcNode = msg->getSrcNode();
	int layer = msg->getLayer();

	MCPOMemberMsg* mcpoMsg = msg->decapsulate<MCPOMemberMsg>();

	logging_info("handling membership reply for layer "
		<< layer << " from " << srcNode.toString());

	MCPOMemberMsg::MemberList list = mcpoMsg->getList();
	typedef vector<NodeID* >::iterator ListIterator;

	logging_info("targetLayer: " << targetLayer);

	/* Check layer response */
	if (layer == targetLayer) {

		ListIterator it  = list.begin();

		/* Use member information for own cluster update */
		for ( ; it != list.end(); it++) {

			NodeID neighbor = *(*it);

			//clusters[0].add(queryRspMsg->getNodes(i));
			clusters[layer].add(neighbor);

			logging_info("Adding: " << neighbor.toString());

		}

		clusters[layer].add(srcNode);

		/* Initiate joining of lowest layer */
		sendJoinRequest( srcNode, layer );

		changeState(READY);

	} else {

		/* Evaluate RTT to queried node */
                query_start = ariba::utility::Helper::getElapsedMillis() - query_start;
		logging_info("Set query_start: " << query_start);

		/* Find out who is nearest cluster member in response,
		 * if nodes are given */
		if (list.size() > 0) {

			ListIterator it  = list.begin();

			for ( ; it != list.end(); it++) {

				NodeID neighbor = *(*it);

				MCPOMsg mcpomsg( MCPOMsg::MCPO_JOINEVAL, myNodeID, layer );
				logging_info("Send JoinEval to: " << neighbor);

				node->sendMessage( &mcpomsg, neighbor, serviceID );

			}


		} else { // Directly query same node again for lower layer

			Query(srcNode, layer-1);
			logging_info("Query: " << srcNode);

		}

		evalLayer = layer;
		logging_info("evalLayer: " << evalLayer);
		query_compare = ariba::utility::Helper::getElapsedMillis();
		logging_info("query_compare: " << query_compare);

	}

	delete mcpoMsg;

} // handleMembershipReply


/** ***************************************************************************
 * Request Join in given Layer
 * @param dst Node to send request to
 * @param layer Layer to join
 *****************************************************************************/
void MCPO::sendJoinRequest( const NodeID& dst, int layer )
{

	logging_info("sending join request for layer "
		<< layer << " to "
		<< dst.toString());

	MCPOMsg mcpomsg( MCPOMsg::MCPO_JOIN_CLUSTER, myNodeID );
	MCPOLayerMsg mcpolayermsg( layer );

	mcpomsg.encapsulate( &mcpolayermsg );

	node->sendMessage( &mcpomsg, dst, serviceID );

	/* Create peer context to leader */
	/* Check if peer info already exists */
	std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(dst);

	if (it != peerInfos.end()) { /* We already know this node */

	} else { /* Create PeerInfo object */

		MCPOPeerInfo* pi = new MCPOPeerInfo(this);

		peerInfos.insert(std::make_pair(dst, pi));

	}

	/* Locally add thisNode, too */
	clusters[layer].add(myNodeID);

	/* Set leader for cluster */
	clusters[layer].setLeader(dst);

	// If not already running, schedule some timers

	heartbeatTimer->start();

	refineTimer->start();

} // sendJoinRequest

/** ***************************************************************************
 * Handle Join Request Message
 * @param msg Join Message to handle
 *****************************************************************************/
void MCPO::handleJoinRequest( MCPOMsg* msg )
{

	NodeID srcNode = msg->getSrcNode();

	MCPOLayerMsg* mcpoMsg
            = const_cast<MCPOMsg*>(msg)->decapsulate<MCPOLayerMsg>();

	int layer = mcpoMsg->getLayer();

	logging_info("handling join request for layer "
		<< layer << " from "
		<< srcNode.toString());

	if (!clusters[layer].getLeader().isUnspecified()) {

		if ( clusters[layer].getLeader() == myNodeID ) {

			clusters[layer].add(srcNode);

			/* Create peer context to joining node */
			/* Check if peer info already exists */
			std::map<NodeID, MCPOPeerInfo*>::iterator it
				= peerInfos.find(srcNode);

			if (it != peerInfos.end()) { /* We already know this node */

			} else { /* Create PeerInfo object */

				MCPOPeerInfo* pi = new MCPOPeerInfo(this);

				peerInfos.insert(std::make_pair(srcNode, pi));

			}

			sendHeartbeats();

		} else {

			delete mcpoMsg;
			return;

		}

	}

	delete mcpoMsg;

} // handleJoinRequest


/** ***************************************************************************
 * Handling link requests
 * @param remote The requesting node
 *****************************************************************************/
bool MCPO::onLinkRequest(const NodeID& remote) {

	return true;

} // onLinkRequest


/** ***************************************************************************
 * Receives Message from Ariba
 * @param msg The received message
 * @param remote The node the message was received from
 * @param link The Link the message was received through
 *****************************************************************************/
void MCPO::onMessage(	const DataMessage& msg,
						const NodeID& remote,
						const LinkID& lnk) {

	logging_debug("receiveMessage");

	if (!isReady) {
	  return;
	  logging_info("!isReady");
	}

	MCPOMsg* mcpoMsg = msg.getMessage()->decapsulate<MCPOMsg>();

	 std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(mcpoMsg->getSrcNode());
	 if (it != peerInfos.end()) {

		 it->second->touch();

	 }

	switch(mcpoMsg->getType()) {

			case MCPOMsg::MCPO_LOOKUP_RP:

				logging_info("Type: MCPO_LOOKUP_RP");

				handleRPSearchMessage( mcpoMsg->getSrcNode() );

				break;

			case MCPOMsg::MCPO_LOOKUP_RP_REPLY:

				logging_info("Type: MCPO_LOOKUP_RP_REPLY");

				handleRPReplyMessage( mcpoMsg->getSrcNode() );

				break;

			case MCPOMsg::MCPO_QUERY:

				logging_info("Type: MCPO_QUERY");

				handleMembershipQuery( mcpoMsg );

				break;

			case MCPOMsg::MCPO_QUERY_RESPONSE:

				logging_info("Type: MCPO_QUERY_RESPONSE");

				handleMembershipReply( mcpoMsg );

				break;

			case MCPOMsg::MCPO_JOIN_CLUSTER:

				logging_info("Type: MCPO_JOIN_CLUSTER");

				handleJoinRequest( mcpoMsg );

				break;

			case MCPOMsg::MCPO_HEARTBEAT:

				logging_info("Type: MCPO_HEARTBEAT");

				handleHeartbeat( mcpoMsg );

				break;

			case MCPOMsg::MCPO_LEADERHEARTBEAT:

				logging_info("Type: MCPO_LEADERHEARTBEAT");

				handleHeartbeat( mcpoMsg );

				break;

			case MCPOMsg::MCPO_LEADERTRANSFER:

				logging_info("Type: MCPO_LEADERTRANSFER");

				handleHeartbeat( mcpoMsg );

				break;

			case MCPOMsg::MCPO_APPDATA:

				logging_debug("Type: MCPO_APPDATA");

				handleGroupData( mcpoMsg );

				break;

			case MCPOMsg::MCPO_BROADCAST_RP:

				logging_info("Type: MCPO_BROADCAST_RP from: " << mcpoMsg->getSrcNode());

				//
				// Compare NodeIDs to figure out RP
				//
				if ((Rendevouz == myNodeID) && (mcpoMsg->getSrcNode() > myNodeID)) {

					// Foreign RP may stay, merge with his cluster
					logging_info("Foreign RP may stay, merge with his cluster");

					// send merge request
					ClusterMergeRequest(mcpoMsg->getSrcNode(), 0 );

					clusters[0].setLeader(mcpoMsg->getSrcNode());

					setRendevouz(mcpoMsg->getSrcNode());

				} else if (Rendevouz != myNodeID) {

					setRendevouz(mcpoMsg->getSrcNode());
					logging_info("Set Rendevouz to: " <<  mcpoMsg->getSrcNode());
					structureTimer->start();

				}

				break;

			case MCPOMsg::MCPO_CLUSTER_MERGE_REQUEST: {

				logging_info("Type: MCPO_CLUSTER_MERGE_REQUEST from: " << mcpoMsg->getSrcNode());

				short layer = mcpoMsg->getLayer();

				MCPOClusterMergeMsg* mergeMsg = mcpoMsg->decapsulate<MCPOClusterMergeMsg>();

				// Only react if I am a leader of this cluster layer

				if (clusters[layer].getLeader().isUnspecified()) {

					delete mergeMsg;

					logging_info(" NO LEADER! BREAK. MCPO_CLUSTER_MERGE_REQUEST finished.");

					return;

				}

				if (clusters[layer].getLeader() == myNodeID) {

					clusters[layer+1].remove(mcpoMsg->getSrcNode());

						if (clusters[layer+1].getLeader() != myNodeID)
							clusters[layer+1].setLeader(mergeMsg->getClusterLeader());

						// if I am alone now, become leader
						if (clusters[layer+1].getSize() == 1) {

							setRendevouz(myNodeID);

							// cancel layer
							clusters[layer+1].clear();

						}


						MCPOClusterMergeMsg::MemberList list = mergeMsg->getList();
						typedef vector<NodeID* >::iterator ListIterator;

						ListIterator it = list.begin();

						logging_info( "Adding " << list.size() << " elements..." );

						for (unsigned int i=0; i<list.size(); i++) {

								NodeID id = *list.at(i);
								logging_info( "Add: " << id );

								/* Add new node to cluster */
								clusters[layer].add(id);

								/* Check if peer info already exists */
								std::map<NodeID, MCPOPeerInfo*>::iterator it
									= peerInfos.find(id);

								if (it != peerInfos.end()) { /* We already know this node */

								} else { /* Create PeerInfo object */

									MCPOPeerInfo* pi = new MCPOPeerInfo(this);

									peerInfos.insert(std::make_pair(id, pi));

								}

						}

						// Finally, add requester
						/* Add new node to cluster */
						clusters[layer].add(mcpoMsg->getSrcNode());
						logging_info( "Adding " << mcpoMsg->getSrcNode() << " to layer " <<  layer );

						/* Check if peer info already exists */
						std::map<NodeID, MCPOPeerInfo*>::iterator it2
							= peerInfos.find(mcpoMsg->getSrcNode());

						if (it2 != peerInfos.end()) { /* We already know this node */

						} else { /* Create PeerInfo object */

							MCPOPeerInfo* pi = new MCPOPeerInfo(this);

							peerInfos.insert(std::make_pair(mcpoMsg->getSrcNode(), pi));

							pi->touch();

						}

						// if node is cluster leader, check size bounds for every cluster
						for (int i=getHighestLayer(); i >= 0; i--) {

							logging_info( "Cluster: " << i << ", size: " << clusters[i].getSize());

							for (int z=0; z < clusters[i].getSize(); z++) {

								long dist = 0;

								std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(z));

								if (it != peerInfos.end()) {

									dist = it->second->get_distance();

								}

								logging_info( "		Member: " << clusters[i].get(z) << ", Distance: " << dist);

							}

							logging_info( "		LEADER: " << clusters[i].getLeader());

						} // All Layers

				} else {// Forward to new cluster leader


				}

				sendHeartbeats();

				delete mergeMsg;

				logging_info("Type: MCPO_CLUSTER_MERGE_REQUEST finished.");

			}

			break;

			case MCPOMsg::MCPO_REMOVE: {

				logging_info("Type: MCPO_REMOVE from: " << mcpoMsg->getSrcNode() << " for layer : " << mcpoMsg->getLayer());

				if (!clusters[mcpoMsg->getLayer()].getLeader().isUnspecified()) {

					if (clusters[mcpoMsg->getLayer()].getLeader() == myNodeID) {

						// check prevents visualization arrows to be deleted by error
						if (clusters[mcpoMsg->getLayer()].contains(mcpoMsg->getSrcNode())) {

							clusters[mcpoMsg->getLayer()].remove(mcpoMsg->getSrcNode());

						}

						//Send proactive heartbeat
						sendHeartbeats();

					}

					if (clusters[mcpoMsg->getLayer()].getLeader() == mcpoMsg->getSrcNode()) {


					}

				}

				}

				break;

			case MCPOMsg::MCPO_JOINEVAL: {

				MCPOMsg mymsg( MCPOMsg::MCPO_JOINEVAL_RESPONSE, myNodeID, mcpoMsg->getLayer());
				logging_info("Send JoinEvalResponse to: " << mcpoMsg->getSrcNode());

				node->sendMessage( &mymsg, mcpoMsg->getSrcNode(), serviceID );

				}

				break;

			case MCPOMsg::MCPO_JOINEVAL_RESPONSE: {

				logging_info("Received JoinEvalResponse from: " << mcpoMsg->getSrcNode());
				logging_info("evalLayer: " << evalLayer);
				logging_info("mcpoMsg->getLayer(): " << mcpoMsg->getLayer());

				logging_info("query_compare: " << query_compare);
				logging_info("query_start: " << query_start);
				logging_info("tempResolver: " << tempResolver);

				if (evalLayer > 0 && evalLayer == mcpoMsg->getLayer()) {

					query_compare = ariba::utility::Helper::getElapsedMillis() - query_compare;

					if (query_compare < query_start) {

						Query(mcpoMsg->getSrcNode(), mcpoMsg->getLayer()-1);

					} else {

						Query(tempResolver, mcpoMsg->getLayer()-1);

					}

					evalLayer = -1;
				}

				}

				break;

			default:

				break;

	}

	delete mcpoMsg;

} // onMessage


/** ***************************************************************************
 * Send data to all MCPO members
 * @param msg The message to be disseminated
 *****************************************************************************/
void MCPO::sendToAll( const DataMessage& msg ) {

	if (!isReady) {
		logging_warn( "sendToAll: !isReady!");
		return;
	}

	logging_debug( "sendToAll layer 0: GROUP SIZE :  " << clusters[0].getSize());

	Message *tempMsg = new Message();
	tempMsg->encapsulate(msg.getMessage());
	tempMsg->setReleasePayload(true);

	Data data = tempMsg->getPayload();

	//delete tempMsg;

	for (int i=0; i<= getHighestLayer(); i++) {

		logging_debug( "...layer: " << i);

		for (int j=0; j<clusters[i].getSize(); j++) {

			logging_debug( "...node: " << clusters[i].get(j) );

			if (clusters[i].get(j) != myNodeID && clusters[i].get(j) != NodeID::UNSPECIFIED) {

			logging_debug( "sendToAll: sending message to node in cluster " << clusters[i].get(j).toString() );

			//Encapsulate message for mcpo distribution
                        MCPOMsg mcpomsg( MCPOMsg::MCPO_APPDATA, myNodeID, i );
        //                 std::cout << "CREATED MCPO-MESSAGE WITH GROUPID: "<< mcpomsg.getGroupID().toString()<<std::endl;
			mcpomsg.setPayload( data.clone() );
			mcpomsg.setReleasePayload(true);
			node->sendMessage( &mcpomsg, clusters[i].get(j), serviceID );

		}

	}

	delete tempMsg;

  }

} // sendToAll


/** ***************************************************************************
 * Send data to all MCPO members of a specific group
 * @param msg The message to be disseminated
 * @param groupID target group
 *****************************************************************************/
void MCPO::sendToGroup( const DataMessage& msg, const ServiceID groupID ) {

    if (!isReady) {
            logging_warn( "sendToAll: !isReady!");
            return;
    }

    logging_debug( "sendToAll layer 0: GROUP SIZE :  " << clusters[0].getSize());

    Message *tempMsg = new Message();
    tempMsg->encapsulate(msg.getMessage());
    tempMsg->setReleasePayload(true);

    Data data = tempMsg->getPayload();
    //delete tempMsg;

    for (int i=0; i<= getHighestLayer(); i++) {

            logging_debug( "...layer: " << i);

            for (int j=0; j<clusters[i].getSize(); j++) {

                    logging_debug( "...node: " << clusters[i].get(j) );

                    if (clusters[i].get(j) != myNodeID && clusters[i].get(j) != NodeID::UNSPECIFIED) {

                    logging_debug( "sendToGroup: sending message to node in cluster " << clusters[i].get(j).toString() );

                    //Encapsulate message for mcpo distribution
                    MCPOMsg mcpomsg( MCPOMsg::MCPO_APPDATA, myNodeID, i, groupID );
               //      std::cout << "CREATED MCPO-MESSAGE WITH GROUPID: "<< mcpomsg.getGroupID().toString()<<std::endl;
                    mcpomsg.setPayload( data.clone() );
                    mcpomsg.setReleasePayload(true);

                    node->sendMessage( &mcpomsg, clusters[i].get(j), serviceID );

            }

    }

    delete tempMsg;

}

} // sendToGroup


/** ***************************************************************************
 * Join specific group per ID
 * @param groupID group to join
 *****************************************************************************/
void MCPO::joinGroup( const ServiceID groupID ) {

	//Check if already joined that group
	bool found = false;
	std::vector< ServiceID >::iterator it;

	for (it = ownGroups.begin(); it != ownGroups.end(); it++) {

		if ( *it == groupID )
			found = true;

	}

	if (!found) {

		ownGroups.push_back( groupID );
		logging_debug( "joined group: " << groupID.toString() );

	}

} // joinGroup


/** ***************************************************************************
 * Leave specific group per ID
 * @param groupID group to join
 *****************************************************************************/
void MCPO::leaveGroup( const ServiceID groupID ) {

	//Check if member of that group
	std::vector< ServiceID >::iterator it;
	for (it = ownGroups.begin(); it != ownGroups.end(); it++) {

        if ( *it == groupID ) {
            //ownGroups.erase(it);
			logging_debug( "left group: " << groupID.toString() );
            break;
		}

	}
    if (it != ownGroups.end())
        ownGroups.erase(it);

} // leaveGroup


/** ***************************************************************************
 * Sends heartbeat to all peers
 *****************************************************************************/
void MCPO::sendHeartbeats()
{

	logging_info( "sending Heartbeats..."
			<< " [spovnetid " << node->getSpoVNetId().toString() << "]"
			<< " and [nodeid " << node->getNodeId().toString() << "]" );

	/* Go through all cluster layers from top to bottom */
	for (int i=getHighestLayer(); i >= 0; i--) {

		/* Determine if node is cluster leader in this layer */
		if (!clusters[i].getLeader().isUnspecified()) {

			if (clusters[i].getLeader() == myNodeID) {

				/* Send Heartbeat to all members in cluster, except me */
				for (int j = 0; j < clusters[i].getSize(); j++) {

					if (clusters[i].get(j) != myNodeID) {

						/* Build heartbeat message with info on all current members */
						MCPOMsg mcpomsg( MCPOMsg::MCPO_LEADERHEARTBEAT, myNodeID, i );
						MCPOLeaderHeartbeatMsg lhbmsg;

						logging_info( "sending LeaderHeartbeat...");

						/* Fill in members */
						for (int k = 0; k < clusters[i].getSize(); k++) {

							NodeID node = clusters[i].get(k);
							lhbmsg.insert( new NodeID( node ) );
				        	//logging_info("Member: " << node);

						}

						/* Fill in distances to members */
						for (int k = 0; k < clusters[i].getSize(); k++) {

							std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(k));

							if (it != peerInfos.end()) {

								lhbmsg.insertDistance(it->second->get_distance());
								//logging_info("Distance: " << it->second->get_distance());

							} else {

								lhbmsg.insertDistance(-1);
								//logging_info("peerInfo unknown: Distance: -1");

							}

						}

						/* Fill in Supercluster members, if existent */
						if (clusters[i+1].getSize() > 0) {

							lhbmsg.setScLeader(clusters[i+1].getLeader());

							for (int j = 0; j < clusters[i+1].getSize(); j++) {

								NodeID node = clusters[i+1].get(j);
								lhbmsg.insertSc( new NodeID( node ));
								//logging_info("SC Member: " << node);

							}

						}

						/* Get corresponding sequence numbers out of peerInfo */
						std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

						if (it != peerInfos.end()) {

							unsigned int seqNo = it->second->get_last_sent_HB();

							lhbmsg.setSeqNo(++seqNo);
							//logging_info("SeqNo: " << seqNo);

							it->second->set_backHB(it->second->get_backHBPointer(), seqNo, ariba::utility::Helper::getElapsedMillis());
							it->second->set_last_sent_HB(seqNo);

							it->second->set_backHBPointer(!it->second->get_backHBPointer());

							lhbmsg.setSeqRspNo(it->second->get_last_recv_HB());
							//logging_info("SeqRspNo: " << it->second->get_last_recv_HB());

							if (it->second->get_last_HB_arrival() > 0) {

								lhbmsg.setHb_delay(ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());
								//logging_info("Hb_delay: " << ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());

							} else {

								lhbmsg.setHb_delay(0);
								//logging_info("Hb_delay: 0");

							}

						}

						mcpomsg.encapsulate(&lhbmsg);

						node->sendMessage( &mcpomsg, clusters[i].get(j), serviceID );
						logging_info( "to : " << clusters[i].get(j).toString());

					}

				}

			} else { // I am normal cluster member

				/* Build heartbeat message with info on all current members */

				/* Send Heartbeat to all members in cluster, except me */
				for (int j = 0; j < clusters[i].getSize(); j++) {

					if (clusters[i].get(j) != myNodeID) {

						MCPOMsg mcpomsg( MCPOMsg::MCPO_HEARTBEAT, myNodeID, i );
						MCPOHeartbeatMsg hbmsg;

						logging_info( "sending Heartbeat...");

						/* Fill in members */
						for (int k = 0; k < clusters[i].getSize(); k++) {

							NodeID node = clusters[i].get(k);
							hbmsg.insert( new NodeID( node ) );
							//logging_info("Member: " << node);

						}

						/* Fill in distances to members */
						for (int k = 0; k < clusters[i].getSize(); k++) {

							std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(k));

							if (it != peerInfos.end()) {

								hbmsg.insertDistance(it->second->get_distance());
								//logging_info("Distance: " << it->second->get_distance());

							} else {

								hbmsg.insertDistance(-1);
								//logging_info("Distance: -1");

							}

						}

						/* Get corresponding sequence number out of peerInfo */
						std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

						if (it != peerInfos.end()) {

							unsigned int seqNo = it->second->get_last_sent_HB();

							hbmsg.setSeqNo(++seqNo);
//							logging_info("SeqNo: " << seqNo);

							it->second->set_backHB(it->second->get_backHBPointer(), seqNo, ariba::utility::Helper::getElapsedMillis());
//							logging_info( "set_backHB : " << ariba::utility::Helper::getElapsedMillis());
							it->second->set_backHBPointer(!it->second->get_backHBPointer());
							it->second->set_last_sent_HB(seqNo);

							hbmsg.setSeqRspNo(it->second->get_last_recv_HB());
//							logging_info("SeqRspNo: " << it->second->get_last_recv_HB());

							hbmsg.setHb_delay(ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());
//							logging_info("Hb_delay: " << ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());


						}

						mcpomsg.encapsulate(&hbmsg);

						node->sendMessage( &mcpomsg, clusters[i].get(j), serviceID );
						logging_info( "to : " << clusters[i].get(j).toString());

					}

				}

			}

		}

	}

	// Additionally, ping all supercluster members, if existent
	if (clusters[getHighestLayer()+1].getSize() > 0) {

		for (int i=0; i<clusters[getHighestLayer()+1].getSize(); i++) {

			if (clusters[getHighestLayer()+1].get(i) != clusters[getHighestLayer()].getLeader()) {

				if (useCLIO) {

					#ifdef HAVE_CLIO_CLIO_H

					//Build CLIOOrder for remote node we received HB from
					CLIOOrder myorder;
					myorder = CLIOOrder(CLIOOrder::RTT, CLIOOrder::ONESHOT, myNodeID, myNodeID, clusters[getHighestLayer()+1].get(i) );

					//Remembers OrderId for later remapping
					clioOrders.insert(std::make_pair(myorder.getOrderID(), clusters[getHighestLayer()+1].get(i)));
					clioContext.insert(std::make_pair(myorder.getOrderID(), CLIOOrder::RTT));

					logging_info( "comitting CLIO Order for SC latency...");
					logging_info( ".....Order(CLIOOrder::RTT, CLIOOrder::ONESHOT, "+myNodeID.toString()+", "+myNodeID.toString()+", "+clusters[getHighestLayer()+1].get(i).toString());

					//Submit order
					try {

						clioNode->commitOrder(myorder, this);

					} catch (spovnet::clio::OrderRejected& e) {

						//Remove OrderID, for we had a response
						clioOrders.erase(myorder.getOrderID());
						clioContext.erase(myorder.getOrderID());

					} catch (spovnet::clio::CLIORpcError& e) {

						//Remove OrderID, for we had a response
						clioOrders.erase(myorder.getOrderID());
						clioContext.erase(myorder.getOrderID());

					}
					#endif

				} else { //Measure autonomously

					MCPOMsg mcpomsg( MCPOMsg::MCPO_PING_PROBE, myNodeID, getHighestLayer()+1 );

					node->sendMessage( &mcpomsg, clusters[getHighestLayer()+1].get(i), serviceID );

					std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[getHighestLayer()+1].get(i));

					if (it != peerInfos.end()) {

						it->second->set_distance_estimation_start(ariba::utility::Helper::getElapsedMillis());

					}

				}

			}

		}

	}

} // sendHeartbeats


/******************************************************************************
 * Send Heartbeat to specified peer
 * @param _node target peer
 * @param layer layer to be written to heartbeat
 *****************************************************************************/
void MCPO::sendHeartbeatTo(const NodeID& _node, int layer)
{

	if (clusters[layer].getLeader() == myNodeID) {

		/* Build heartbeat message with info on all current members */
		MCPOMsg mcpomsg( MCPOMsg::MCPO_LEADERHEARTBEAT, myNodeID, layer );
		MCPOLeaderHeartbeatMsg lhbmsg;

		/* Fill in members */
		for (int k = 0; k < clusters[layer].getSize(); k++) {

			NodeID mynode = clusters[layer].get(k);
			lhbmsg.insert( new NodeID( mynode ) );

		}

		/* Fill in distances to members */
		for (int k = 0; k < clusters[layer].getSize(); k++) {

			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[layer].get(k));

			if (it != peerInfos.end()) {

				lhbmsg.insertDistance(it->second->get_distance());

			} else {

				lhbmsg.insertDistance(0);

			}

		}

		/* Fill in Supercluster members, if existent */
		if (clusters[layer+1].getSize() > 0) {

			lhbmsg.setScLeader(clusters[layer+1].getLeader());

			for (int j = 0; j < clusters[layer+1].getSize(); j++) {

				NodeID mynode = clusters[layer+1].get(j);
				lhbmsg.insertSc( new NodeID( mynode ));

			}

		}

		/* Get corresponding sequence numbers out of peerInfo */
		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(_node);

		if (it != peerInfos.end()) {

			unsigned int seqNo = it->second->get_last_sent_HB();

			lhbmsg.setSeqNo(++seqNo);

			it->second->set_backHB(it->second->get_backHBPointer(), seqNo, ariba::utility::Helper::getElapsedMillis());
			it->second->set_last_sent_HB(seqNo);

			it->second->set_backHBPointer(!it->second->get_backHBPointer());

			lhbmsg.setSeqRspNo(it->second->get_last_recv_HB());

			if (it->second->get_last_HB_arrival() > 0) {

				lhbmsg.setHb_delay(ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());

			} else {

				lhbmsg.setHb_delay(0);

			}

		}

		mcpomsg.encapsulate(&lhbmsg);

		node->sendMessage( &mcpomsg, _node, serviceID );

	} else {

		/* Build heartbeat message with info on all current members */

		/* Send Heartbeat to all members in cluster, except me */
		MCPOMsg mcpomsg( MCPOMsg::MCPO_HEARTBEAT, myNodeID, layer );
		MCPOHeartbeatMsg hbmsg;

		/* Fill in members */
		for (int k = 0; k < clusters[layer].getSize(); k++) {

			NodeID node = clusters[layer].get(k);
			hbmsg.insert( new NodeID( node ) );

		}

		/* Fill in distances to members */
		for (int k = 0; k < clusters[layer].getSize(); k++) {

			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[layer].get(k));

			if (it != peerInfos.end()) {

				hbmsg.insertDistance(it->second->get_distance());

			} else {

				hbmsg.insertDistance(-1);

			}

		}

		/* Get corresponding sequence number out of peerInfo */
		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(_node);

		if (it != peerInfos.end()) {

			unsigned int seqNo = it->second->get_last_sent_HB();

			hbmsg.setSeqNo(++seqNo);

			it->second->set_backHB(it->second->get_backHBPointer(), seqNo, ariba::utility::Helper::getElapsedMillis());
			it->second->set_backHBPointer(!it->second->get_backHBPointer());
			it->second->set_last_sent_HB(seqNo);

			hbmsg.setSeqRspNo(it->second->get_last_recv_HB());

			hbmsg.setHb_delay(ariba::utility::Helper::getElapsedMillis() - it->second->get_last_HB_arrival());

		}

		mcpomsg.encapsulate(&hbmsg);

		node->sendMessage( &mcpomsg, _node, serviceID );

	}

} // sendHeartbeatTo


/******************************************************************************
 * Handles incoming heartbeat messages
 * @param msg Heartbeat message
 *****************************************************************************/
void MCPO::handleHeartbeat( MCPOMsg* msg )
{
	// First, check if heartbeat is LeaderTransfer
	if (msg->getType() == MCPOMsg::MCPO_LEADERTRANSFER) {

		MCPOLeaderHeartbeatMsg* hbMsg = msg->decapsulate<MCPOLeaderHeartbeatMsg>();

		logging_info( "Handle LeaderTransfer from: " << msg->getSrcNode() << " for layer " << msg->getLayer());

		if (!clusters[msg->getLayer()].getLeader().isUnspecified()) {

			logging_info( "clusters[msg->getLayer()].getLeader(): " << clusters[msg->getLayer()].getLeader());

			/* React only if I am not already leader */
			if (clusters[msg->getLayer()].getLeader() != myNodeID) {

				logging_info( "clusters[msg->getLayer()].getLeader() != myNodeID ");
				clusters[msg->getLayer()].clear();
				logging_info( "clusters[hbMsg->getLayer()].clear(); ");
				clusters[msg->getLayer()].setLeader(myNodeID);
				logging_info( "clusters[hbMsg->getLayer()].setLeader(myNodeID); ");

				logging_info( "I am new leader of cluster " << msg->getLayer() );

				MCPOLeaderHeartbeatMsg::MemberList list = hbMsg->getList();
				typedef vector<NodeID* >::iterator ListIterator;

				ListIterator it = list.begin();

				logging_info( "Adding " << list.size() << " elements..." );

				//while (it != list.end()) {
				for (unsigned int i=0; i<list.size(); i++) {

					//NodeID id = **it;
					NodeID id = *list.at(i);
					logging_info( "Add: " << id );

					clusters[msg->getLayer()].add(id);

					std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(id);

					if (it != peerInfos.end()) { /* We already know this node */

						it->second->touch();

					} else {

						//We don't know him yet

					}

					//it++;

				}

				MCPOLeaderHeartbeatMsg::MemberList sclist = hbMsg->getScList();

				if (sclist.size() > 0) {

					clusters[msg->getLayer()+1].clear();

					ListIterator itsc = sclist.begin();

					while (itsc != sclist.end()) {

						NodeID id = **itsc;

						logging_info( "Add SC: " << id );

						clusters[msg->getLayer()+1].add(id);

						std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(id);

						if (it != peerInfos.end()) { /* We already know this node */

							it->second->touch();

						} else {

							//We don't know him yet

						}

						itsc++;

					}

					clusters[msg->getLayer()+1].add(myNodeID);

					clusters[msg->getLayer()+1].setLeader(hbMsg->getScLeader());

					if ((clusters[msg->getLayer()+1].getLeader() == myNodeID) && (clusters[msg->getLayer()+2].getSize() == 0)) {

						setRendevouz(myNodeID);
						logging_info( "setRendevouz(myNodeID)");

					} else {

						sendJoinRequest(hbMsg->getScLeader(), msg->getLayer()+1);

					}

				} else {

					setRendevouz(myNodeID);
					logging_info( "setRendevouz(myNodeID)");

				}

			}

			clusters[msg->getLayer()].set_Last_LT();

		} else {

			logging_info( "clusters[msg->getLayer()].getLeader() is unspecified!");

		}

		delete hbMsg;

		return;

	} else if (msg->getType() == MCPOMsg::MCPO_HEARTBEAT) {

		MCPOHeartbeatMsg* hbMsg = msg->decapsulate<MCPOHeartbeatMsg>();

		logging_info( "Handle Heartbeat from: " << msg->getSrcNode());

		/* Update sequence number information and evaluate distance */
		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(msg->getSrcNode());

		if (it != peerInfos.end()) { /* We already know this node */

				/* CLIO usage for demonstrator: On HB initiate ONESHOT latency measurement */

				if (useCLIO) {

					#ifdef HAVE_CLIO_CLIO_H

					//Build CLIOOrder for remote node we received HB from
					CLIOOrder myorder;
					myorder = CLIOOrder(CLIOOrder::RTT, CLIOOrder::ONESHOT, myNodeID, myNodeID, msg->getSrcNode() );

					//Remembers OrderId for later remapping
					clioOrders.insert(std::make_pair(myorder.getOrderID(), msg->getSrcNode()));
					clioContext.insert(std::make_pair(myorder.getOrderID(), CLIOOrder::RTT));

					logging_info( "comitting CLIO Order for latency...");
					logging_info( ".....Order(CLIOOrder::RTT, CLIOOrder::ONESHOT, "+myNodeID.toString()+", "+myNodeID.toString()+", "+msg->getSrcNode().toString());

					//Submit order
					try {

						clioNode->commitOrder(myorder, this);

					} catch (spovnet::clio::OrderRejected& e) {

						//Remove OrderID, for we had a response
						clioOrders.erase(myorder.getOrderID());
						clioContext.erase(myorder.getOrderID());

					} catch (spovnet::clio::CLIORpcError& e) {

						//Remove OrderID, for we had a response
						clioOrders.erase(myorder.getOrderID());
						clioContext.erase(myorder.getOrderID());

					}
					#endif

				} else {

					it->second->set_last_HB_arrival(ariba::utility::Helper::getElapsedMillis());
					logging_info( "set_last_HB_arrival: " << ariba::utility::Helper::getElapsedMillis());

					if (it->second->get_backHB(hbMsg->getSeqRspNo()) > 0) {

						/* Valid distance measurement, get value */
						long oldDistance = it->second->get_distance();

						logging_info( "oldDistance = it->second->get_distance(): " << it->second->get_distance());

						/* Use Exponential Moving Average with factor 0.1 */
						unsigned long newDistance = (double)(ariba::utility::Helper::getElapsedMillis() - it->second->get_backHB(hbMsg->getSeqRspNo()) - hbMsg->getHb_delay())/2.0;
						logging_info( "ariba::utility::Helper::getElapsedMillis(): " << ariba::utility::Helper::getElapsedMillis());
						logging_info( "it->second->get_backHB(hbMsg->getSeqRspNo(): " << it->second->get_backHB(hbMsg->getSeqRspNo()));
						logging_info( "hbMsg->getHb_delay(): " << hbMsg->getHb_delay());
						logging_info( "newDistance: " << newDistance);

						if (oldDistance > 0) {

							it->second->set_distance((unsigned long)((0.1 * (double)newDistance) + (0.9 * (double)oldDistance)));
							logging_info( "set_distance weighted: " << (unsigned long)((0.1 * (double)newDistance) + (0.9 * (double)oldDistance)));

						} else {

							it->second->set_distance((unsigned long)newDistance);
							logging_info( "set_distance: " << (unsigned long)newDistance);

						}

					}

				}

			it->second->set_last_recv_HB(hbMsg->getSeqNo());

			it->second->touch();

			MCPOHeartbeatMsg::MemberList list = hbMsg->getList();
			MCPOHeartbeatMsg::DistanceList distlist = hbMsg->getDistanceList();
			typedef vector<NodeID* >::iterator ListIterator;

//			logging_info( "list.size(): " << list.size());

			for (unsigned int j=0; j<list.size(); j++) {

				it->second->updateDistance(*list.at(j), (long)distlist.at(j)-1);
//				logging_info( "updateDistance: " << *list.at(j) << " : " << (long)(distlist.at(j)-1));

			}

		}

		delete hbMsg;

	} else if (msg->getType() == MCPOMsg::MCPO_LEADERHEARTBEAT) {

		MCPOLeaderHeartbeatMsg* hbMsg = msg->decapsulate<MCPOLeaderHeartbeatMsg>();

		logging_info( "Handle LeaderHeartbeat from: " << msg->getSrcNode());

		leaderHeartbeats.push_back(std::make_pair(msg->getSrcNode(), ariba::utility::Helper::getElapsedMillis()));

		if (leaderHeartbeats.size() > 3) {

			unsigned long predecessor =  leaderHeartbeats.at(leaderHeartbeats.size()-2).second;

			if (ariba::utility::Helper::getElapsedMillis() < (predecessor + 5000)) {

				if (leaderHeartbeats.at(leaderHeartbeats.size()-2).first != msg->getSrcNode()) {

					if (leaderHeartbeats.at(leaderHeartbeats.size()-3).first == msg->getSrcNode()) {

						if (leaderHeartbeats.at(leaderHeartbeats.size()-4).first == leaderHeartbeats.at(leaderHeartbeats.size()-2).first) {

							logging_info( "DER KANDIDAT !");

							//TESTING
							MCPOMsg mcpomsg( MCPOMsg::MCPO_REMOVE, myNodeID, msg->getLayer() );

							node->sendMessage( &mcpomsg, leaderHeartbeats.at(leaderHeartbeats.size()-2).first, serviceID );

						}

					}

				}

			}

		}

		/* Tidy up leaderheartbeats */
		if (leaderHeartbeats.size() > 4) {

			for(unsigned int i=0; i<(leaderHeartbeats.size()-4); i++) {

				leaderHeartbeats.erase(leaderHeartbeats.begin());

			}

		}

		/* Update sequence number information and evaluate distance */
		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(msg->getSrcNode());

		if (it != peerInfos.end()) { /* We already know this node */

			if (useCLIO) {

				#ifdef HAVE_CLIO_CLIO_H

				//Build CLIOOrder for remote node we received HB from
				CLIOOrder myorder;
				myorder = CLIOOrder(CLIOOrder::RTT, CLIOOrder::ONESHOT, myNodeID, myNodeID, msg->getSrcNode() );

				//Remembers OrderId for later remapping
				clioOrders.insert(std::make_pair(myorder.getOrderID(), msg->getSrcNode()));
				clioContext.insert(std::make_pair(myorder.getOrderID(), CLIOOrder::RTT));

				logging_info( "comitting CLIO Order for latency...");
				logging_info( ".....Order(CLIOOrder::RTT, CLIOOrder::ONESHOT, "+myNodeID.toString()+", "+myNodeID.toString()+", "+msg->getSrcNode().toString());

				//Submit order
				try {

					clioNode->commitOrder(myorder, this);

				} catch (spovnet::clio::OrderRejected& e) {

					//Remove OrderID, for we had a response
					clioOrders.erase(myorder.getOrderID());
					clioContext.erase(myorder.getOrderID());

				} catch (spovnet::clio::CLIORpcError& e) {

					//Remove OrderID, for we had a response
					clioOrders.erase(myorder.getOrderID());
					clioContext.erase(myorder.getOrderID());

				}
				#endif

			} else {

				it->second->set_last_HB_arrival(ariba::utility::Helper::getElapsedMillis());
				logging_info( "set_last_HB_arrival: " << ariba::utility::Helper::getElapsedMillis());

				if (it->second->get_backHB(hbMsg->getSeqRspNo()) > 0) {

					/* Valid distance measurement, get value */
					logging_info( "hbMsg->getSeqRspNo(): " << hbMsg->getSeqRspNo());
					logging_info( "it->second->get_backHB(hbMsg->getSeqRspNo()): " << it->second->get_backHB(hbMsg->getSeqRspNo()));
					logging_info( "hbMsg->getHb_delay(): " << hbMsg->getHb_delay());

					it->second->set_distance((ariba::utility::Helper::getElapsedMillis() - it->second->get_backHB(hbMsg->getSeqRspNo()) - hbMsg->getHb_delay())/2);
					logging_info( "set_distance: " << ((ariba::utility::Helper::getElapsedMillis() - it->second->get_backHB(hbMsg->getSeqRspNo()) - hbMsg->getHb_delay())/2));

				}

				it->second->set_last_recv_HB(hbMsg->getSeqNo());
				logging_info( "hbMsg->getSeqNo(): " << hbMsg->getSeqNo());

			}

		}

		it = peerInfos.find(msg->getSrcNode());

		if (it != peerInfos.end()) {

			MCPOHeartbeatMsg::MemberList list = hbMsg->getList();
//			logging_info( "list.size(): " << list.size());
			MCPOHeartbeatMsg::DistanceList distlist = hbMsg->getDistanceList();

			for (unsigned int j=0; j<list.size(); j++) {

				it->second->updateDistance(*list.at(j), (long)(distlist.at(j))-1);
//				logging_info( "updateDistance: " << *list.at(j) << " : " << (long)(distlist.at(j))-1);

			}

		}

		// Maintain cluster memberships

		if (!clusters[msg->getLayer()].contains(myNodeID)) {

			/* Node is not part of this cluster, remove it */
			logging_info( "Node is not part of this cluster, remove it!!!");

			//TESTING
			MCPOMsg mcpomsg( MCPOMsg::MCPO_REMOVE, myNodeID, msg->getLayer() );

			node->sendMessage( &mcpomsg, msg->getSrcNode(), serviceID );

			delete hbMsg;

			return;

		}

		if (clusters[msg->getLayer()].getLeader() == myNodeID) {

			logging_info( "clusters[msg->getLayer()].getLeader() == myNodeID");

			if (ariba::utility::Helper::getElapsedMillis() < clusters[msg->getLayer()].get_Last_LT() + 1000) {

				delete hbMsg;

				return;

			}

			/* Winner: The one with minimum max distance*/
			bool allIn = true;

			//Check if we're talking about same cluster
			MCPOHeartbeatMsg::MemberList list = hbMsg->getList();

			for(unsigned int u=0; u<list.size(); u++) {

				if (!clusters[msg->getLayer()].contains(*list.at(u))) {
					allIn = false;

				} else {

				}

			}

			if (allIn) {

				// Perform check for better cluster leader
				TaSet cl;
				for (int l=0; l<clusters[msg->getLayer()].getSize(); l++) {

					cl.insert(clusters[msg->getLayer()].get(l));

				}

				unsigned long myDistance = getMaxDistance(myNodeID, cl);
				unsigned long hisDistance = getMaxDistance(msg->getSrcNode(), cl);

				if (myDistance > hisDistance) {

					TaSet cl;
					for (int i=0; i<clusters[msg->getLayer()].getSize(); i++) {

						cl.insert(clusters[msg->getLayer()].get(i));

					}

					LeaderTransfer(msg->getLayer(), msg->getSrcNode(), cl, clusters[msg->getLayer()+1].getLeader());

					clusters[msg->getLayer()].setLeader(msg->getSrcNode());

					gracefulLeave(msg->getLayer());

				} else {

					sendHeartbeatTo(msg->getSrcNode(), msg->getLayer());
					logging_info( "sendHeartbeatTo(msg->getSrcNode(), msg->getLayer())");

					delete hbMsg;

					return;

				}

			} else { // We have different children, simply leave other

				//logging_info( "We have different children, simply leave other!!!");

				//TESTING
				//MCPOMsg mcpomsg( MCPOMsg::MCPO_REMOVE, myNodeID, msg->getLayer() );

				//node->sendMessage( &mcpomsg, msg->getSrcNode(), serviceID );

				//return;

			}

		}

		/* Everything is in order. Process HB */

		logging_info( "Everything is in order. Process HB.");

		NodeID tempID = msg->getSrcNode();

		// Restart StructureTimer!
		if (structureTimer->isRunning()) {
			structureTimer->reset();
			logging_info( "structureTimer->reset();");
		} else {
			structureTimer->start();
			logging_info( "structureTimer->start();");
		}

		//TODO Experimental
		for (int m=msg->getLayer(); m<10; m++) {

			clusters[m].clear();

		}

		MCPOHeartbeatMsg::MemberList list = hbMsg->getList();

		for(unsigned int i=0; i<list.size(); i++) {

			//Check if member is already part of cluster

			/* Check if peer info already exists */
			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(*list.at(i));

			if (it != peerInfos.end()) { /* We already know this node */

			} else { /* Create PeerInfo object */

				MCPOPeerInfo* pi = new MCPOPeerInfo(this);

				// TODO Experimental
				pi->set_last_HB_arrival(ariba::utility::Helper::getElapsedMillis());

				peerInfos.insert(std::make_pair(*list.at(i), pi));

			}

			clusters[msg->getLayer()].add(*list.at(i));
			logging_info( "...add: " << *list.at(i));

		}

		// set cluster leadership
		clusters[msg->getLayer()].setLeader(msg->getSrcNode());

		MCPOLeaderHeartbeatMsg::MemberList sclist = hbMsg->getScList();

		if (sclist.size() > 0) {

			clusters[msg->getLayer()+1].clear();

			for(unsigned int i=0; i<sclist.size(); i++) {

				/* Check if peer info already exists */
				std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(*sclist.at(i));

				if (it != peerInfos.end()) { /* We already know this node */

				} else { /* Create PeerInfo object */

					MCPOPeerInfo* pi = new MCPOPeerInfo(this);

					pi->set_last_HB_arrival(ariba::utility::Helper::getElapsedMillis());

					peerInfos.insert(std::make_pair(*sclist.at(i), pi));

				}

				clusters[msg->getLayer()+1].add(*sclist.at(i));

			}

			clusters[msg->getLayer()+1].setLeader(hbMsg->getScLeader());

			MCPOHeartbeatMsg::DistanceList distlist = hbMsg->getDistanceList();

			it = peerInfos.find(msg->getSrcNode());

			if (it != peerInfos.end()) {

				for (unsigned int k=0; k<list.size(); k++) {

					it->second->updateDistance(*list.at(k), distlist.at(k));

				}

			} else {

				MCPOPeerInfo* pi = new MCPOPeerInfo(this);

				pi->set_last_HB_arrival(ariba::utility::Helper::getElapsedMillis());

				peerInfos.insert(std::make_pair(msg->getSrcNode(), pi));

			}
		}

		logging_info( "done.");

		delete hbMsg;

	}


} // handleHeartbeat


/** ***************************************************************************
 * Handle incoming group data
 * @param msg the data message to handle
 *****************************************************************************/
void MCPO::handleGroupData( MCPOMsg* msg ) {

	logging_debug( "handleGroupData : ");

	Data value = msg->getPayload();
    DataMessage dmsg(value.getBuffer(),value.getLength());

	ServiceID group = msg->getGroupID();

	//Check if message has group restrictions
        if (!msg->getGroupID().isUnspecified()) {

		//Only receive if I am member of that group
		std::vector< ServiceID >::iterator it;

		for (it = ownGroups.begin(); it != ownGroups.end(); it++) {

			if ( *it == group ) {

                receiver->receiveData( dmsg, msg->getGroupID() );

                break;
			}

		}

	} else {
        receiver->receiveData( *msg );

	}

	int sourceLayer = msg->getLayer();
	logging_debug( "...sourceLayer: " << sourceLayer);
//
	for (int layer=0; clusters[layer].contains(myNodeID); layer++) {

		logging_debug( "...layer: " << layer);

		if ( sourceLayer != layer ) {

			for (int j=0; j<clusters[layer].getSize(); j++) {

				logging_debug( "...clusters[layer].getSize(): " << clusters[layer].getSize());

				if (clusters[layer].get(j) != myNodeID && clusters[layer].get(j) != NodeID::UNSPECIFIED) {

					MCPOMsg mcpomsg( MCPOMsg::MCPO_APPDATA, myNodeID, layer, group );

					mcpomsg.setPayload( value.clone() );

					logging_debug( "Forwarding APP DATA to : " << clusters[layer].get(j));

					node->sendMessage( &mcpomsg, clusters[layer].get(j), serviceID );

				}
			}

		}

	}

} // handleGroupData


/******************************************************************************
 * Calculate maximum distance to a given set of peers
 * @param member Peer to be source of calculation
 * @param neighbors Set of peers
 *****************************************************************************/
unsigned long MCPO::getMaxDistance(NodeID member, std::set<NodeID> neighbors)
{
	unsigned long maxDelay = 0;
	unsigned long delay = 0;

	if (member == myNodeID) {

		std::set<NodeID>::iterator it = neighbors.begin();

		while (it != neighbors.end()) {

			std::map<NodeID, MCPOPeerInfo*>::iterator it2 = peerInfos.find(*it);

			if (it2 != peerInfos.end()) {

				delay = it2->second->get_distance();
				maxDelay = std::max(delay, maxDelay);

			}

			it++;

		}

	} else {

		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(member);

		if (it != peerInfos.end()) {

			std::set<NodeID>::iterator it2 = neighbors.begin();

			while (it2 != neighbors.end()) {

				delay = it->second->getDistanceTo(*it2);
				maxDelay = std::max(delay, maxDelay);

				it2++;

			}

		}

	}

	return maxDelay;

} // getMaxDistance


/******************************************************************************
 * Calculate the mean distance between a set of peers
 * @param neigbors The set of peers
 *****************************************************************************/
unsigned long MCPO::getMeanDistance(std::set<NodeID> neighbors)
{
	unsigned long meanDelay = 0;
	unsigned long delay = 0;
	unsigned int number = 0;

	std::set<NodeID>::iterator it = neighbors.begin();

	while (it != neighbors.end()) {

		if (*it != myNodeID) {

			std::map<NodeID, MCPOPeerInfo*>::iterator it2 = peerInfos.find(*it);

			if (it2 != peerInfos.end()) {

				delay = it2->second->get_distance();

				if  (delay > 0) {

					meanDelay += delay;
					number++;

				}

			}

		}

		it++;

	}

	if (number > 0) {

		return meanDelay/number;

	} else {

		return 0;

	}

} // getMeanDistance


/******************************************************************************
 * Transer cluster leadership to a new leader
 * @param layer Layer to be transferred
 * @param leader New leader
 * @param cluster Set of child nodes to be transferred
 * @param sc_leader Current supercluster leader
 *****************************************************************************/
void MCPO::LeaderTransfer(int layer, NodeID leader, TaSet cluster, NodeID sc_leader)
{

	/* Build heartbeat message with info on all current members */
	MCPOMsg mcpomsg( MCPOMsg::MCPO_LEADERTRANSFER, myNodeID, layer );
	MCPOLeaderHeartbeatMsg lhbmsg;

	logging_info( "LeaderTransfer to : " << leader);
	logging_info( "Layer : " << layer);

	// fill in members
	logging_info( "cluster.size() : " << cluster.size());
	TaSetIt it = cluster.begin();
	int i = 0;
	while (it != cluster.end()) {
		NodeID node = *it;
		lhbmsg.insert( new NodeID( node ) );
		logging_info( "Member : " << node);
		it++;
	}


	/* Fill in Supercluster members, if existent */
	if (clusters[i+1].getSize() > 0) {

		lhbmsg.setScLeader(clusters[i+1].getLeader());

		for (int j = 0; j < clusters[i+1].getSize(); j++) {

			NodeID node = clusters[i+1].get(j);
			logging_info( "SCMember : " << node);
			lhbmsg.insertSc( new NodeID( node ));

		}

	} else {

		lhbmsg.setScLeader(leader);
		logging_info( "SCLeader : " << leader);

	}

	mcpomsg.encapsulate(&lhbmsg);

	node->sendMessage( &mcpomsg, leader, serviceID );

} // LeaderTransfer


/******************************************************************************
 * Leave cluster gracefully
 * @param bottomLayer
 *****************************************************************************/
void MCPO::gracefulLeave(short bottomLayer)
{

	for (int i=9; i>bottomLayer; i--) {

		if (clusters[i].getSize() > 0) {

			if (clusters[i].contains(myNodeID)) {

    			if (clusters[i].getLeader() == myNodeID) {

					Remove(i);

					if (clusters[i].getSize() > 0) {

						TaSet cl;
						for (int j=0; j<clusters[i].getSize(); j++) {

							cl.insert(clusters[i].get(j));

						}

						NodeID new_sc_center = findCenter(cl).first;

						if (new_sc_center.isUnspecified()) {

							new_sc_center = clusters[i].get(0);

						}

						clusters[i].setLeader(new_sc_center);

						LeaderTransfer(i, new_sc_center, cl, new_sc_center);

						// repair RP
						if (i == 9) {

							Rendevouz = new_sc_center;

						} else if (clusters[i+1].getSize() == 0) {

							Rendevouz = new_sc_center;

						}

					}

    			} else {

    				// simply leave cluster

    				Remove(i);
    				clusters[i].remove(myNodeID);

    			}

			}

		}

	}

} // gracefulLeave


/******************************************************************************
 * Remove peer from cluster
 * @param layer Layer to be removed from
 *****************************************************************************/
void MCPO::Remove(int layer)
{

	logging_info( "Remove from layer : " << layer);

	MCPOMsg mcpomsg( MCPOMsg::MCPO_REMOVE, myNodeID, layer );

	node->sendMessage( &mcpomsg, clusters[layer].getLeader(), serviceID );

	clusters[layer].remove(myNodeID);

} // Remove


/******************************************************************************
 * Calculate center of a cluster
 * @param cluster Set of peers
 * @param allowRandom If no result is possible, allow random determination
 * @result std::pair The center peer and its minimum distance to all peers
 *****************************************************************************/
std::pair<NodeID,unsigned long> MCPO::findCenter(TaSet cluster, bool allowRandom) {

	TaSet::const_iterator it = cluster.begin();
	NodeID center = NodeID::UNSPECIFIED;
	unsigned long min_delay = 1000;

	if (cluster.size() > 1) {

		while (it != cluster.end()) {

				unsigned long delay = getMaxDistance(*it, cluster);

				if ((delay > 0) && (delay < min_delay)) {

					min_delay = delay;
					center = *it;

				}

			it++;
		}

	}

	if (center.isUnspecified()) {
		center = *(cluster.begin());
	}

	return std::make_pair(center, min_delay);

} // findCenter


/******************************************************************************
 * Calculate center of a cluster
 * @param cluster Set of peers
 * @param allowRandom If no result is possible, allow random determination
 * @result std::pair The center peer and its minimum distance to all peers
 *****************************************************************************/
std::pair<NodeID, unsigned long> MCPO::findCenter(std::vector<NodeID> cluster, bool allowRandom)
{
	TaSet clusterset;
	std::vector<NodeID>::const_iterator it = cluster.begin();

	while(it != cluster.end()) {
		clusterset.insert(*it);
		it++;
	}
	return findCenter(clusterset, allowRandom);

} // findCenter


/******************************************************************************
 * The main maintenance function
 *****************************************************************************/
void MCPO::maintenance()
{

	logging_info( "maintenance: " << myNodeID);
	logging_info( "TIME: " << ariba::utility::Helper::getElapsedMillis());
	logging_info("RP: " << Rendevouz);

	//Handle Visualization Inconcistencies

	if (useVisuals) {

		for (int i=0; i<10; i++) {

			if (clusters[i].getLeader().isUnspecified() || !clusters[i].contains(myNodeID))
				continue;

			NodeID tempID = clusters[i].getLeader();

			if (clusters[i].getLeader() == myNodeID) { // I am leader, have no links

				if (visLinks[i].size() > 0) {

					//disconnect
					for (unsigned int j=0; j < visLinks[i].size(); j++) {

						logging_info( "Delete VisLink to ... " << visLinks[i].at(j).toString());
						visual.visDisconnect(visualIdMcpo, myNodeID, visLinks[i].at(j), string(""));

					}

					visLinks[i].clear();

				} else {


				}


			} else { // I am not leader, check for draws links

				if (visLinks[i].size() == 0) {

					//connect
					logging_info( "Create VisLink to ... " << tempID.toString());
					visual.visConnect(visualIdMcpo, myNodeID, tempID, string(""));
					visLinks[i].push_back(clusters[i].getLeader());

				} else { // there are already links drawn

					for (unsigned int j=0; j < visLinks[i].size(); j++) {

						if (visLinks[i].at(j) != tempID) {

							logging_info( "Delete VisLink to ... " << visLinks[i].at(j).toString());
							visual.visDisconnect(visualIdMcpo, myNodeID, visLinks[i].at(j), string(""));

						}

					}

					visLinks[i].clear();
					visLinks[i].push_back(tempID);

					visual.visConnect(visualIdMcpo, myNodeID, tempID, string(""));

				}

			}

		}

	}

	// care for structure connection timer
	if (!Rendevouz.isUnspecified()) {

		if (Rendevouz == myNodeID) {
		  if (structureTimer->isRunning()) {
			structureTimer->stop();
		  }

			if (!useDHT) {
				logging_info("sending RP broadcast message via BO Broadcast");

				MCPOMsg mcpomsg( MCPOMsg::MCPO_BROADCAST_RP, myNodeID );

				node->sendBroadcastMessage( &mcpomsg, serviceID );
			} else {

				if (!(rpcollisionTimer->isRunning())) {
					rpcollisionTimer->reset();
					rpcollisionTimer->start();
					logging_info( "rpCollisionTimer->start()");
				}

			}
			//Fix for Demo:
			for (int i=getHighestLayer(); i >= 0; i--) {

				clusters[i].setLeader(myNodeID);

			}

		} else {


			if (structureTimer->isRunning()) {
				//structureTimer->reset();
				logging_info( "structureTimer->isRunning()");
			} else {
				structureTimer->start();
				logging_info( "structureTimer->start();");
			}

			if (rpcollisionTimer->isRunning()) {
				rpcollisionTimer->stop();
				logging_info( "rpCollisionTimer->stop()");
			}

        }
	} else {

		setRendevouz(myNodeID);
        //structureTimer->stop();
	}

	/* Delete deprecated tempPeers from map */

	bool deleted;

	do {

		deleted = false;

		std::map<NodeID, unsigned long>::iterator it = tempPeers.begin();

		while (it != tempPeers.end()) {

			if (ariba::utility::Helper::getElapsedMillis() > (it->second + 15000)) {

					logging_info( "ariba::utility::Helper::getElapsedMillis(): " << ariba::utility::Helper::getElapsedMillis());
					logging_info( "it->second: " << it->second);
					tempPeers.erase(it->first);
					deleted = true;
					logging_info( "ERASE 1: " << it->first);
					break;

			}

			it++;

		}

	} while (deleted);

	/* Delete nodes that haven't been active for too long autonomously */

	std::map<NodeID, MCPOPeerInfo*>::iterator it2 = peerInfos.begin();

	while (it2 != peerInfos.end()) {

		if (it2->first != myNodeID) {

			unsigned long offset = peerTimeoutInterval*1000;

			if (ariba::utility::Helper::getElapsedMillis() > (it2->second->getActivity() + offset)) {

				logging_info( "ariba::utility::Helper::getElapsedMillis(): " << ariba::utility::Helper::getElapsedMillis());
				logging_info( "it2->second->getActivity(): " << it2->second->getActivity());
				logging_info( "offset: " << offset);

				logging_info( "ERASE 2: " << it2->first);

				// Delete node from all layer clusters
				for (int i=0; i<10; i++) {

					if (clusters[i].contains(it2->first)) {

						clusters[i].remove(it2->first);

						/* If node was leader, elect new! */
						if (!(clusters[i].getLeader().isUnspecified())) {

							if (clusters[i].getLeader() == it2->first) {

								logging_info( "Node was leader! Elect new!");

								// Perform check for new cluster leader
								TaSet cl;
								for (int l=0; l<clusters[i].getSize(); l++) {

									cl.insert(clusters[i].get(l));

								}

								NodeID new_leader = findCenter(cl).first;

								/* Remove old leader from supercluster */
								clusters[i+1].remove(clusters[i].getLeader());

								clusters[i].setLeader(new_leader);

								if (new_leader == myNodeID) {

									logging_info( "I am new leader.");

									// I am new leader
									if (clusters[i+1].getSize() > 0) {

										/* TODO Experimental Hack: If old leader is also leader of sc -> basicJoinLayer*/
										if (clusters[i+1].getLeader() == clusters[i].getLeader()) {

											/* Locally add thisNode, too */
											clusters[i+1].add(myNodeID);

											BasicJoinLayer(i+1);

										} else {

											sendJoinRequest(clusters[i+1].getLeader(), i+1);

										}

									} else {

										setRendevouz(myNodeID);
										logging_info( "setRendevouz(myNodeID)");

									}

								sendHeartbeats();

								}

							}

						}

					}

				}

				NodeID cand = it2->first;
				++it2;
				peerInfos.erase(cand);
				continue;

			}

		}

		it2++;

	}

	std::vector<NodeID> members;
	// if node is cluster leader, check size bounds for every cluster
	for (int i=getHighestLayer(); i >= 0; i--) {

		logging_info( "Cluster: " << i << ", size: " << clusters[i].getSize());

		for (int z=0; z < clusters[i].getSize(); z++) {

			long dist = 0;

			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(z));

			if (it != peerInfos.end()) {

				dist = it->second->get_distance();

			}

			logging_info( "		Member: " << clusters[i].get(z) << ", Distance: " << dist);

			members.push_back(clusters[i].get(z));

		}

		logging_info( "		LEADER: " << clusters[i].getLeader());

		//TODO Experimental Check for inconsistency: If I am node in cluster but not leader in subcluster, remove
		if (clusters[i].contains(myNodeID) && (i > 0)) {

			if (clusters[i-1].getLeader() != myNodeID) {

				Remove(i);
				return;

			}

		}

		if (!clusters[i].getLeader().isUnspecified()) {

			if (clusters[i].getLeader() == myNodeID) {

				if (clusters[i].getSize() > (3*k-1)) {

					ClusterSplit(i);

					return;

				}


				if ((clusters[i].getSize() < k) && (clusters[i+1].getSize() > 1)) {

					ClusterMerge(i);

					return;

				} else if ((clusters[i].getSize() < k)) {

					// TODO Close highest layer due to no other members!

				}

			}

		}

	} // All Layers

	// if highest super cluster has more than one member
	if (clusters[getHighestLayer()+1].getSize() > 1) {

		std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[getHighestLayer()].getLeader());

		if (it != peerInfos.end()) {

			if (it->second->get_distance() > 0) {

				unsigned long distance = it->second->get_distance() - ((it->second->get_distance()/100) * SC_PROC_DISTANCE);

				unsigned long smallest = 10000;
				NodeID candidate = NodeID::UNSPECIFIED;

				for (int i=0; i < clusters[getHighestLayer()+1].getSize(); i++) {

					if (clusters[getHighestLayer()+1].get(i) != clusters[getHighestLayer()].getLeader()) {

						std::map<NodeID, MCPOPeerInfo*>::iterator it2 = peerInfos.find(clusters[getHighestLayer()+1].get(i));

						if (it2 != peerInfos.end()) {

							if ((it2->second->get_distance() < (long)smallest) && (it2->second->get_distance() > 0)) {
								smallest = it2->second->get_distance();
								candidate = it2->first;

							}

						}

					}

				}

				std::set<NodeID> clusterset;

				for (int m=0; m<clusters[getHighestLayer()+1].getSize(); m++)
				{

					clusterset.insert(clusters[getHighestLayer()+1].get(m));

				}

				unsigned long meanDistance = getMeanDistance(clusterset);

				unsigned long minCompare = (meanDistance/100)*SC_MIN_OFFSET;

				logging_info( "	meanDistance: " << meanDistance );
				logging_info( "	minCompare: " << minCompare );

				logging_info( "	smallest: " << smallest );
				logging_info( "	distance: " << distance );

				if ((smallest < distance) && ((distance - smallest) > minCompare)) { // change supercluster

					logging_info( "	change supercluster!: " );

					short highestLayer = getHighestLayer();

					// leave old
					Remove(highestLayer);

					// join new
					sendJoinRequest(candidate, highestLayer);

					return;

				}

			}

		} else {

			// TODO Do nothing?

		}

	}

	for (int i=getHighestLayer(); i >= 0; i--) {

		if (clusters[i].getSize() > 1 && clusters[i].getLeader() == myNodeID) {

			bool allDistancesKnown = true;

			/* Only make decisions if node has total distance knowledge in this cluster */
			for (int j=0; j<clusters[i].getSize(); j++) {

				/* Check if peer info already exists */
				std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(clusters[i].get(j));

				if (it != peerInfos.end()) { /* We already know this node */

					unsigned long distance = it->second->get_distance();

					if (distance < 0) {
						allDistancesKnown = false;
						continue;
					}

					for (int k=0; k<clusters[i].getSize(); k++) {

						if ((it->first != myNodeID) && (clusters[i].get(k) != it->first)) {

							if (it->second->getDistanceTo(clusters[i].get(k)) < 0) {
								allDistancesKnown = false;
								break;
							}
						}

					}

				} else { /* Create PeerInfo object */

					allDistancesKnown = false;

				}

			}

			if (allDistancesKnown) {

				// Perform check for better cluster leader
				TaSet cl;
				for (int l=0; l<clusters[i].getSize(); l++) {

					cl.insert(clusters[i].get(l));

				}

				NodeID new_leader = findCenter(cl).first;

				std::set<NodeID> clusterset;

				for (int m=0; m<clusters[i].getSize(); m++)
				{

					clusterset.insert(clusters[i].get(m));

				}

				unsigned long meanDistance = getMeanDistance(clusterset);
				logging_info( "meanDistance: " << meanDistance);
				unsigned long oldDistance = getMaxDistance(clusters[i].getLeader(), clusterset);
				logging_info( "oldDistance: " << oldDistance);
				unsigned long newDistance = getMaxDistance(new_leader, clusterset);
				logging_info( "newDistance: " << newDistance);
				unsigned long compareDistance = (oldDistance - ((oldDistance/100)*30));
				logging_info( "compareDistance: " << newDistance);

				unsigned long minCompare = (meanDistance/100)*5;
				logging_info( "minCompare: " << minCompare);

				//Hack to avoid fluctuations
				if (minCompare < 50)
					minCompare = 50;

				logging_info( "minCompare: " << minCompare);

				if ((newDistance < compareDistance) && ((compareDistance - newDistance) > minCompare)) {

					logging_info( "LEADERCHANGE !!! to " << new_leader);

					if (new_leader != myNodeID) {

						gracefulLeave(i);

						LeaderTransfer(i, new_leader, cl, new_leader);

						if (clusters[i+1].getSize() == 0)
							setRendevouz(new_leader);


					} else {

						/* I am the new leader of this cluster */
						/* Check if I already was*/
						if (clusters[i].getLeader() == myNodeID) {

							// Do nothing

						} else {

							/* Remove old leader from supercluster */
							clusters[i+1].remove(clusters[i].getLeader());

							// I am new leader
							if (clusters[i+1].getSize() > 0) {

								/* TODO Experimental Hack: If old leader is also leader of sc -> basicJoinLayer*/
								if (clusters[i+1].getLeader() == clusters[i].getLeader()) {

									/* Locally add thisNode, too */
									clusters[i+1].add(myNodeID);

									BasicJoinLayer(i+1);

								} else {

                                    sendJoinRequest(clusters[i+1].getLeader(), i+1);

								}

							} else {

								setRendevouz(myNodeID);

							}

						}

					}

					// Set new leader for this cluster
					clusters[i].setLeader(new_leader);
                    std::cout << "highest layer: " << getHighestLayer()<< std::endl;
                    std::cout << "leader: " << new_leader.toString()<< std::endl;

				} else {

                    std::cout << "highest layer: " << getHighestLayer()<< std::endl;

                    std::cout << "leader: " << getParent().toString()<< std::endl;
				//return;

				}

			}

		} // getSize() > 1

	}

} // maintenance


/******************************************************************************
 * Split a given cluster
 * @param layer Layer cluster to be split
 *****************************************************************************/
void MCPO::ClusterSplit(int layer)
{

	/* Get cluster to be splitted */
	MCPOCluster cluster = clusters[layer];

	/* Introduce some helper structures */
	std::vector<NodeID> vec1;
	std::vector<NodeID> vec2;
	std::vector<NodeID> cl1;
	std::vector<NodeID> cl2;
	NodeID cl1_center = NodeID::UNSPECIFIED;
	NodeID cl2_center = NodeID::UNSPECIFIED;
	unsigned long min_delay = 999;

	for (int i=0; i<cluster.getSize(); i++) {

		/* Put all members to first vector */
		vec1.push_back(cluster.get(i));

		/* Put first half of members to second vector */
		if (i < cluster.getSize()/2) {

			vec2.push_back(cluster.get(i));

		}

	}

	int combinations = 0;

	TaSet cl1set, cl2set, newClSet;
	TaSet::iterator sit;

	if (cluster.getSize() < (6*k-1)) {

		/* Go through all combinations of clusters */
		do {

			combinations++;

			/* Introduce some helper structures */
			NodeID q1_center;
			NodeID q2_center;
			std::vector<NodeID> vec3;

			/* Determine elements that are in first set but not in second */
			std::set_difference(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), inserter(vec3, vec3.begin()));

			unsigned long min_q1_delay = 999999;
			unsigned long min_q2_delay = 999999;
			unsigned long max_delay = 0;

			q1_center = findCenter(vec2).first;

			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(q1_center);

			if (it != peerInfos.end()) {

				min_q1_delay = it->second->get_distance();

			} else {

				min_q1_delay = 0;

			}

			q2_center = findCenter(vec3).first;

			it = peerInfos.find(q2_center);

			if (it != peerInfos.end()) {

				min_q2_delay = it->second->get_distance();

			} else {

				min_q2_delay = 0;

			}

			max_delay = std::max(min_q1_delay, min_q2_delay);

			if (min_delay == 0) min_delay = max_delay;

			if ((max_delay < min_delay) && !q1_center.isUnspecified() && !q2_center.isUnspecified()) {

				min_delay = max_delay;
				cl1 = vec2;
				cl2 = vec3;
				cl1_center = q1_center;
				cl2_center = q2_center;

			}

		} while(next_combination(vec1.begin(), vec1.end(), vec2.begin(), vec2.end()));

	//	std::cout << "COMBINATIONS: " << combinations << std::endl;

		//build sets
		std::vector<NodeID>::iterator vit;

		vit = cl1.begin();
		while(vit != cl1.end()) {
			cl1set.insert(*vit);
			logging_info("Set 1 : " << *vit);
			vit++;
		}

		vit = cl2.begin();
		while(vit != cl2.end()) {
			cl2set.insert(*vit);
			logging_info("Set 2 : " << *vit);
			vit++;
		}

	} else {

		//std::cout << "SIZE > 6k-1 !!! " << endl;

	}

	//if no valid split possible, split randomly
	if (cl1_center.isUnspecified() || cl2_center.isUnspecified()) {

	//	std::cout << myNodeID << " RANDOM SPLIT" << std::endl;

		cl1set.clear();
		cl2set.clear();

		for (int i=0; i<cluster.getSize(); i++) {

			if (i < cluster.getSize()/2) {

				cl1set.insert(cluster.get(i));

			} else {

				cl2set.insert(cluster.get(i));
			}

		}

		cl1_center = findCenter(cl1set,true).first;
		logging_info("cl1_center : " << cl1_center);
		cl2_center = findCenter(cl2set,true).first;
		logging_info("cl2_center : " << cl2_center);

	}

	//find new neighbors
	NodeID newLeader, otherLeader;
	TaSet newCl;
	TaSet::iterator it = cl1set.begin();

	while (it != cl1set.end()) {

		if (*it == myNodeID) {

			newCl = cl1set;
			newLeader = cl1_center;
			otherLeader = cl2_center;

		}

		it++;

	}

	it = cl2set.begin();

	while (it != cl2set.end()) {

		if (*it == myNodeID) {

			newCl = cl2set;
			newLeader = cl2_center;
			otherLeader = cl1_center;

		}

		it++;

	}

//####################################################################

    // Cluster split accomplished, now handling consequences

    // CASE 1: We lost all cluster leaderships
    // repair all cluster layer, top down
    if ((cl1_center != myNodeID) && (cl2_center != myNodeID)) {

    	logging_info("We lost all cluster leaderships");

    	gracefulLeave(layer);

    	if (clusters[layer+1].getSize() == 0) {

    		clusters[layer+1].add(cl1_center);
			clusters[layer+1].add(cl2_center);
			clusters[layer+1].setLeader(cl1_center);
			Rendevouz = cl1_center;

    	}

        LeaderTransfer(layer, cl1_center, cl1set, cl1_center);
        LeaderTransfer(layer, cl2_center, cl2set, cl1_center);

    } // CASE 1


    // CASE 2: We stay leader in one of the new clusters
    if ((cl1_center == myNodeID) || (cl2_center == myNodeID)) {

    	 logging_info("We stay leader in one of the new clusters");

		if (clusters[layer+1].getSize() == 0) {

		    clusters[layer+1].add(cl1_center);
			clusters[layer+1].add(cl2_center);

			clusters[layer+1].setLeader(myNodeID);

		}

    	if (cl1_center == myNodeID) {

    		 clusters[layer+1].add(cl2_center);
    		 LeaderTransfer(layer, cl2_center, cl2set, cl1_center);

    	} else {

    		 clusters[layer+1].add(cl1_center);
    		 LeaderTransfer(layer, cl1_center, cl1set, cl1_center);

    	}


    } // CASE 2

	// update local cluster information for focussed layer
    logging_info("update local cluster information for focussed layer");
	TaSet::const_iterator cit = cl1set.begin();
	bool found = false;

	while (cit != cl1set.end()) {

		if (*cit == myNodeID)
			found = true;

	    cit++;

	}

	clusters[layer].clear();

	if (found) {

		clusters[layer].setLeader(cl1_center);

		cit = cl1set.begin();

		while (cit != cl1set.end()) {

			clusters[layer].add(*cit);
			logging_info("clusters[layer].add(*cit) : " << layer << " : " << *cit);

			cit++;

		}

	} else {

		clusters[layer].setLeader(cl2_center);

		cit = cl2set.begin();

		while (cit != cl2set.end()) {
			clusters[layer].add(*cit);
			logging_info("clusters[layer].add(*cit) : " << layer << " : " << *cit);
		    cit++;
		}

	}

	logging_info("ClusterSplit done.");

} // ClusterSplit


/******************************************************************************
 * Merge to clusters in given layer
 * @param layer Layer clusters should be merged in
 *****************************************************************************/
void MCPO::ClusterMerge(int layer)
{

	logging_info("ClusterMerge for layer: " << layer);

	unsigned long min_delay = 999999;

	NodeID min_node = NodeID::UNSPECIFIED;

	for (int i=0; i<clusters[layer+1].getSize(); i++) {

		NodeID node = clusters[layer+1].get(i);

		if (node != myNodeID) {

			std::map<NodeID, MCPOPeerInfo*>::iterator it = peerInfos.find(node);

			if (it != peerInfos.end()) {

				unsigned long delay = it->second->get_distance();

				if ((delay > 0) && (delay < min_delay)) {

					min_delay = delay;
					min_node = node;

				}
			}
		}

	}

	if (!min_node.isUnspecified()) {

		// leave
		for (int i=9; i>layer; i--) {

			if (clusters[i].getSize() > 0) {

				if (clusters[i].contains(myNodeID)) {

		        	if (clusters[i].getLeader() == myNodeID) {

						//Remove(i);

		        		TaSet cl;
		        		for (int j=0; j<clusters[i].getSize(); j++) {

		        			cl.insert(clusters[i].get(j));

		        		}

		        		NodeID new_sc_center = findCenter(cl).first;

		        		clusters[i].setLeader(new_sc_center);

		        		LeaderTransfer(i, new_sc_center, cl, new_sc_center);

		        		// repair RP
		        		if (i == 9) {

		        			Rendevouz = new_sc_center;

		        		} else if (clusters[i+1].getSize() == 0) {

		        			Rendevouz = new_sc_center;

		        		}

		        	} else {

		        		// simply leave cluster
		        		//Remove(i);
		        		clusters[i].remove(myNodeID);

		        	}

		    	}

		    }

		}

		clusters[layer+1].remove(myNodeID);

		// send merge request
		ClusterMergeRequest(min_node, layer);

	} else {

	}

	logging_info("Done.");

} // ClusterMerge


/******************************************************************************
 * Request a cluster merge
 * @param _node Target peer to send request to
 * @param layer Layer to be merged
 *****************************************************************************/
void MCPO::ClusterMergeRequest(const NodeID& _node, int layer)
{

	/* Build heartbeat message with info on all current members */
	MCPOMsg mcpomsg( MCPOMsg::MCPO_CLUSTER_MERGE_REQUEST, myNodeID, layer );
	MCPOClusterMergeMsg mrgmsg( clusters[layer+1].getLeader() );

	/* Fill in members */
	for (int k = 0; k < clusters[layer].getSize(); k++) {

		NodeID node = clusters[layer].get(k);
		mrgmsg.insert( new NodeID( node ) );

	}


	clusters[layer].setLeader(_node);

	mcpomsg.encapsulate( &mrgmsg );

	logging_info("Sending ClusterMergeRequest for layer " << layer << " to " << _node);

    node->sendMessage( &mcpomsg, _node, serviceID );

}

const NodeID& MCPO::getParent()
{
    if(ownGroups.empty())return NodeID::UNSPECIFIED;
    else return clusters->getLeader();
}

 std::vector< NodeID> MCPO::getChildren()
{


    std::vector< NodeID> result;
     if(ownGroups.empty())return result;
    //getting through layers and collecting children
    short highest_layer=getHighestLayer();

    for(short layer=0; layer<highest_layer; layer++){

        int clustersize=clusters[layer].getSize();

        for(int n=0;n<clustersize;n++){
        const NodeID& latest_node=clusters[layer].get(n);

            if(latest_node != myNodeID){
                result.push_back(latest_node);
            }

        }
    }
    return result;
}

// ClusterMergeRequest


/******************************************************************************
 * Startup call
 ******************************************************************************/
void MCPO::startup()
{

} // startup


/******************************************************************************
 * Shutdown call
 *****************************************************************************/
void MCPO::shutdown()
{

	logging_info("shutting down MCPO...");

	//Leaving structure
	gracefulLeave(0);

} // shutdown


/******************************************************************************
 * Reconnect after partitioning
 *****************************************************************************/
void MCPO::reconnectToStructure() {

	logging_info( "reconnectToStructure()!");

	if (Rendevouz.isUnspecified()) {

		setRendevouz(myNodeID);
		return;

	}

	if (Rendevouz == myNodeID)
		return;

	//Demo Fix: Always join lowest layer
	//BasicJoinLayer(getHighestLayer());
	BasicJoinLayer(0);

	structureTimer->start();

} // reconnectToStructure


#ifdef HAVE_CLIO_CLIO_H

/******************************************************************************
 * Handle result from CLIO request
 * @param result The result of measurement
 *****************************************************************************/
void MCPO::onResult(const CLIOResult& result) {

	logging_info( "Got CLIO result:");

	//Got a result from CLIO, get corresponding ID
    OrderID orderId = result.getOrderID();
    logging_info( "....OrderID: " << orderId.toString());

    //Check if we have pending orders with that ID, otherweise ignore result
    std::map<OrderID, NodeID>::iterator it = clioOrders.find(orderId);

    if (it != clioOrders.end()) { /* We found a pending measurement */

    	logging_info( "....FOUND.");

        //Check for errors
        CLIOResult::ErrorCode e = result.getErrorCode();
        if (e == CLIOResult::NO_ERROR) {

        	//Get type of measurement out of map
		    std::map<OrderID, std::string>::iterator it2 = clioContext.find(orderId);

		    std::string myresult = result.getStringResult();
		    int res;
		    res = atoi(myresult.c_str());

		    if (it2->second == CLIOOrder::RTT) { // RTT Measurement

		    	logging_info( "....NodeID: " << it->second.toString());
				logging_info( "....Latency: " << res);
				NodeID candidate = it->second;

				std::map<NodeID, MCPOPeerInfo*>::iterator it3 = peerInfos.find(candidate);

				if (it3 != peerInfos.end()) { /* We already know this node */

					/* Valid distance measurement, get value */
					it3->second->set_distance(res/1000);

				} else {

					//Do nothing

				}
		    }

        } else {

        	//do not react to errors

		}

        //Remove OrderID, for we had a response
        clioOrders.erase(it);
        clioContext.erase(orderId);

    } else {

    	// No order stored with that ID, ignore result
        logging_info( "....NOT FOUND.");

    }

} // onResult


/******************************************************************************
 * CLIO Order has been discarded
 * @param result The result of measurement
 *****************************************************************************/
void MCPO::onOrderDiscarded(const CLIOResult& result) {

} // onOrderDiscarded


/******************************************************************************
 * CLIO Order has finished
 * @param id the ID of the CLIO order
 *****************************************************************************/
void MCPO::onOrderFinished(const OrderID& id) {

} // onOrderFinished


/******************************************************************************
 * CLIO Order has been rejected
 * @param result The result of measurement
 *****************************************************************************/
void MCPO::onOrderRejected(const CLIOResult& result) {

} // onOrderRejected

#endif


/******************************************************************************
 * DHT RP Search request has returned a result
 * @param key The DHT key
 * @param value The DHT result vector
 *****************************************************************************/
void MCPO::onKeyValue( const Data& key, const vector<Data>& value ) {

	logging_info( "GOT DHT RESULT." );

	if (value.size() == 0)
		return;

	Identifier a = Identifier::sha1("MCPO_RP");
	Identifier b;
	data_deserialize(b, key);

	if (a == b) {

		//check if more than one RP is specified in the vector
		if (value.size() > 1) {

			//TODO Handle inconsistency

		}

		//Get value out of vector
		NodeID rp;
		Data myvalue = value[0];
		data_deserialize(&rp, myvalue);

		logging_info( "RESULT:" << rp);

		if (!rp.isUnspecified()) {
			handleRPReplyMessage(rp);
		} else {
			logging_info( "unspec.");
		}

	}

} // onKeyValue


/******************************************************************************
 * Check for existence of RP
 *****************************************************************************/
void MCPO::checkforRP() {

	logging_info( "Check for RP collision...");

	Identifier key = Identifier::sha1("MCPO_RP");
    //node->get(data_serialize(key), serviceID);

    //for ariba 0.8.1
    dht->get(key.toString());

} // checkforRP


}}}
