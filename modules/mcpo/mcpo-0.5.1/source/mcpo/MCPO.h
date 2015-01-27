// [License]
// The Ariba-Underlay Copyright
//
// Copyright (c) 2008-2009, Institute of Telematics, Karlsruhe Institute of Technology (KIT)
//
// Institute of Telematics
// Karlsruhe Institute of Technology (KIT)
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
 * @file MCPO.h
 * @author Christian Huebsch
 */

#ifndef __MCPO_H_
#define __MCPO_H_

namespace ariba {
namespace services {
namespace mcpo {

class MCPO;
class MCPOPeerInfo;

}}}

//#include "config.h"

#include "ariba/ariba.h"

#include "MCPOCluster.h"
#include "timer/MCPOTimer.h"
#include "MCPOPeerInfo.h"

#include "combination.h"

#include "messages/MCPOMsg.h"
#include "messages/MCPOLayerMsg.h"
#include "messages/MCPOMemberMessage.h"
#include "messages/MCPOHeartbeat.h"
#include "messages/MCPOLeaderHeartbeat.h"
#include "messages/MCPOLeaderTransfer.h"
#include "messages/MCPOClusterMergeMsg.h"

#include "ariba/utility/visual/DddVis.h"
#include "ariba/utility/visual/ServerVis.h"
//for ariba 0.8.1
#include "services/ariba_dht/Dht.h"


#ifdef HAVE_CLIO_CLIO_H
	#include "clio/CLIO.h"
	/**
	* CLIO using
	*/
	using spovnet::clio::CLIONode;
	using spovnet::clio::CLIOOrder;
	using spovnet::clio::CLIOResult;
	using spovnet::clio::AppListener;
	using spovnet::clio::OrderID;
#endif


using namespace ariba;
using ariba::utility::Timer;

namespace ariba {
namespace services {
namespace mcpo {

#define visual 				ariba::utility::DddVis::instance()
#define visualIdMcpo	 	ariba::utility::ServerVis::NETWORK_ID_MCPO

enum ProtocolStates {
	INIT,
	BOOTSTRAP,
	READY
};


/******************************************************************************
 *
  *  * \addtogroup public
 * @{
 * The MCPO main class
 *
 *
 * This class implements the group communication component MCPO in SpoVNet, as
 * specified in the architectural documents.
 *****************************************************************************/
class MCPO :
	public CommunicationListener,
#ifdef HAVE_CLIO_CLIO_H
	public AppListener,
#endif
	public Timer {


friend class HeartbeatTimer;
friend class BootstrapTimer;
friend class RefineTimer;
friend class BackoffTimer;
friend class StructureTimer;
friend class RPCollisionTimer;
friend class IPMTimer;
friend class MCPOPeerInfo;

use_logging_h(MCPO);

/******************************************************************************
 * Constructor, Destructor, nested Receiver class definition
 *****************************************************************************/
public:

	/**
	 Specifies a callback interface that will receive all
	 user data that was received from a registered or created group.
	 This does not include control traffic but only user data.
	*/
	class ReceiverInterface {

		public:

		/**
		 This is the callback function that provides the application
		 with multicast user data. All user data from groups that the
		 client has registered will arrive through this function.

		 @param msg The DataMessage object that has been received
		*/
        virtual	void receiveData( const DataMessage& msg ) = 0;
        virtual	void receiveData( const DataMessage& msg, ServiceID sID ) = 0;

		/** For indication of the service being ready to send multicast data */
		virtual	void serviceIsReady() = 0;

	};

	/** Constructor
	 * @param receiver A pointer to the caller implementing the callback functions above
	 * @param ariba A pointer to the ariba substrate
	 * @param node A pointer to the actual node object
	 * @param clioNode A pointer to the CLIO module
	 * @param useVisuals Boolean value to indicate if visualizations should be used
	 */
	//MCPO( ReceiverInterface* receiver, AribaModule *_ariba, Node *_node, CLIONode *_clioNode=NULL, bool _useVisuals=false );

	MCPO( ReceiverInterface* receiver, ServiceID sID, AribaModule *_ariba, Node *_node, /*CLIONode *_clioNode,*/ bool _useVisuals=false );

	MCPO( ReceiverInterface* receiver, AribaModule *_ariba, Node *_node, /*CLIONode *_clioNode,*/ bool _useVisuals=false );


	/** Destructor **/
	~MCPO();

	/** Methods for sending to groups and joins/leaves */
    void sendToAll( const DataMessage& msg );
	void sendToGroup( const DataMessage& msg, const ServiceID groupID );
	void joinGroup( const ServiceID groupID );
	void leaveGroup( const ServiceID groupID );

	/** Startup wrapper interface */
	virtual void startup();
	virtual void shutdown();
    const NodeID &getParent();
    std::vector<NodeID> getChildren();


private:

		unsigned int SC_PROC_DISTANCE; /* percentaged value that a supercluster leader has to be nearer than the current in order to change */
		unsigned int SC_MIN_OFFSET; /* minimum distance offset between two supercluster leaders to determine benefit in change */

		/* switches to indicate components to be used */
		bool useDHT;
		bool useCLIO;
		bool useVisuals;

		/* indicating ready state of MCPO */
		volatile bool isReady;

		/* Define set of TransportAdresses and iterator */
		typedef std::set<NodeID> TaSet;
		typedef std::set<NodeID>::iterator TaSetIt;

		/* Preventing errors in visualization*/
		std::vector<NodeID> visLinks[10];

		/** Receiver interface, implemented by the application,
			to receive multicast user data **/
		ReceiverInterface* receiver;

		/**
		 * Normally, MCPO will hold its current Rendesvouz Point (RP) e.g. in a DHT,
		 * provided by the SpoVNet Base. As a first approach,
		 * the RP is hold locally and updated globally through control traffic
		 **/
		NodeID Rendevouz;

		/** Own NodeID for use in protocol **/
		NodeID myNodeID;

		/* the ariba module and a node */
		AribaModule* ariba;
		Node*        node;
    //for ariba 0.8.1
        ariba_service::dht::Dht* dht;

		/** MCPO ServiceID */
		ServiceID serviceID;

		/** MCPO Cluster information **/
		MCPOCluster *clusters;

		/** Heartbeat Timer **/
		HeartbeatTimer* heartbeatTimer;

		/** Refine Timer **/
		RefineTimer* refineTimer;

		/** Query Timer **/
		QueryTimer* queryTimer;

		/** Backoff Timer **/
		BackoffTimer* backoffTimer;

		/** RPCollision Timer **/
		RPCollisionTimer* rpcollisionTimer;

		/** distance to first queried node that answers */
		unsigned long query_compare;

		/** Structure Timer **/
		StructureTimer* structureTimer;

		/** Bootstrap Timer for RP Serach Timeouts **/
		BootstrapTimer* bootstrapTimer;

		/** k value, indicating allowed cluster size --> k to 3k-1 **/
		int k;

		/** Layer intended to join */
		short targetLayer;

		/** The current queried node */
		NodeID tempResolver;

		/** Holds the query start time for RTT evaluation */
		unsigned long query_start;

		/** Holds the current layer we query, if we do */
		int evalLayer;
		int joinLayer;

		/** Map for all peer infos */
		std::map<NodeID, MCPOPeerInfo*> peerInfos;

		/** Double leader resolution */
		unsigned long first_HB;
		NodeID first_leader;
		unsigned long second_HB;
		NodeID second_leader;

		/** maintain list of received leader heartbeats to detect collisions */
		std::vector<std::pair<NodeID, unsigned long> > leaderHeartbeats;

		/* set holding temporary peered joiner nodes */
		std::map<NodeID, unsigned long> tempPeers;
		bool isTempPeered;

		/** determine when a peer should be regarded gone */
		unsigned long peerTimeoutInterval;

		/** group the local MCPO instance is member of */
		std::vector< ServiceID > ownGroups;

		#ifdef HAVE_CLIO_CLIO_H

			/** CLIO connector */
			CLIONode* clioNode;
			std::map<OrderID, NodeID> clioOrders;
			std::map<OrderID, std::string> clioContext;
		#endif

		/** Initializing local MCPO instance **/
		void initializeMCPO();

		/** reconnect after lost connection to MCPO */
		void reconnectToStructure();

		/**
		 * Set protocol state
	     * @param state State
		 **/
		void changeState( int state );

	    /** Initiate RP Search via BO Broadcast **/
		void sendRPSearchMessage();

		/** Re-Sets the Rendevouz Point */
		void setRendevouz( NodeID id );

		/** checks for RP **/
		void checkforRP();

		/**
		 * Handle RP Seach Message
		 * @param srcNode Node that sent Search message
		 **/
		void handleRPSearchMessage( NodeID srcNode );

	    /**
	     * Initiate RP Search Reply
	     * @param dest NodeID to Reply to
	     **/
		void sendRPReplyMessage( NodeID dest );

		/**
		 * Handle RP Reply Message
		 * @param rendevouz Replying Node
		 **/
		void handleRPReplyMessage( NodeID rendevouz );

	   	/** Joining the hierarchy from scratch */
	    void BasicJoinLayer(short layer);

	    /** Request sublayer information from a cluster leader */
	    void Query(const NodeID& node, short layer);

		/**
		 * Handle Membership Query
		 * @param msg Message to handle
		 **/
		void handleMembershipQuery( MCPOMsg* msg );

		/** Find highest layer in which node is leader */
		short getHighestLeaderLayer();

		/** Find highest layer in which node resides */
	   	short getHighestLayer();

		/**
		 * Handle Membership Reply
		 * @param msg Message to handle
		 **/
		void handleMembershipReply( MCPOMsg* msg );

		/** Request Join in given Layer
		 * @param dst Node to send reply to
		 * @param layer Layer to give information about
		 **/
		void sendJoinRequest( const NodeID& dst, int layer );

		/**
		 * Handle Join Request
		 * @param msg Message to handle
		 **/
		void handleJoinRequest( MCPOMsg* msg );

	   	/** Sends heartbeats to all clusters the node resides in */
	   	void sendHeartbeats();

		/** Handles heartbeats */
		void handleHeartbeat( MCPOMsg* msg );

		/** Handles group data */
	   	void handleGroupData( MCPOMsg* msg );

	   	/** Determines the max distance of a member in a cluster */
	   	unsigned long getMaxDistance(NodeID member, std::set<NodeID> neighbors);

	   	/** Determines the mean distance of a cluster */
	   	unsigned long getMeanDistance(std::set<NodeID> neighbors);

	   	/** Initiates a LeaderTransfer message */
	   	void LeaderTransfer(int layer, NodeID leader, TaSet cluster, NodeID sc_leader);

	   	/** Leaves structure gracefully */
	   	void gracefulLeave(short bottomLayer);

	   	/** Removes node from cluster */
	   	void Remove(int layer);

	   	/** Determines the center of a cluster */
		std::pair<NodeID,unsigned long> findCenter(TaSet cluster, bool allowRandom = false);
		std::pair<NodeID, unsigned long> findCenter(std::vector<NodeID> cluster, bool allowRandom = false);

	   	/** Sends Heartbeat to specific member */
	   	void sendHeartbeatTo(const NodeID& _node, int layer);

		/** Periodic maintenance of the protocol hierarchy */
	   	void maintenance();

	    /** Splits a cluster */
	    void ClusterSplit(int layer);

	   	/** Merges two clusters */
	   	void ClusterMerge(int layer);

	   	/** Sends a ClusterMerge request message */
	   	void ClusterMergeRequest(const NodeID& _node, int layer);



protected:

		/** communication listener interface */
		virtual bool onLinkRequest(const NodeID& remote);
		virtual void onMessage(const DataMessage& msg, const NodeID& remote, const LinkID& lnk= LinkID::UNSPECIFIED);

		virtual void onLinkUp(const LinkID& lnk, const NodeID& remote) {};
		virtual void onLinkDown(const LinkID& lnk, const NodeID& remote) {};
		virtual void onLinkChanged(const LinkID& lnk, const NodeID& remote) {};
		virtual void onLinkFail(const LinkID& lnk, const NodeID& remote) {};
		virtual void onLinkQoSChanged(const LinkID& lnk, const NodeID& remote, const LinkProperties& prop) {};

		virtual void onKeyValue( const Data& key, const vector<Data>& value );
#ifdef HAVE_CLIO_CLIO_H
		/** CLIO interface */
		void onResult(const CLIOResult& result);
		void onOrderDiscarded(const CLIOResult& result);
		void onOrderFinished(const OrderID& id);
		void onOrderRejected(const CLIOResult& result);
#endif

};


}}}

/** @} */

#endif // __MCPO_H_
