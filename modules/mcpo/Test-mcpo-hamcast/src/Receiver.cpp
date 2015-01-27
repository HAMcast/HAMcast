#include "include/Receiver.h"
#include "ariba/ariba.h"
#include "ariba/utility/configuration/Configuration.h"
#include "ariba/utility/visual/DddVis.h"
#include "ariba/utility/logging/Logging.h"
#include "mcpo/messages/MCPOMsg.h"
#include "ariba/DataMessage.h"

using ariba::services::mcpo::MCPO;
using ariba::utility::Configuration;
using ariba::utility::StartupInterface;
using namespace ariba;

namespace ariba{
namespace application {


// logging
//use_logging_cpp( Receiver );

// construction
Receiver::Receiver(string nodename, string endpoint, string bootstrap, string spovnetName, uint32_t portnr)
    :name(nodename),
      m_endpoint(endpoint),
      m_bootstrap(bootstrap),
      m_spovnetName(spovnetName),
      i(1),
      port(portnr)
{
    Timer::setInterval(1000);
}

// destruction
Receiver::~Receiver() {
}

void Receiver::startup2(AribaModule* ariba, string nodename, string endpoint, string bootstrap, string spovnetName, uint32_t portnr ) {
    // set the global log level to info
    //  logging_classlevel_info("MCPO");

    logging_rootlevel_debug();


    std::cout<< "Receiver-"<<nodename<<": starting up Receiver service ... " << std::endl;

    // create ariba module
    logging_debug( "Receiver-"<<nodename<<": creating ariba underlay module ... " );

    Name nodeName = Name::UNSPECIFIED;
    this->name = string(nodename);
    this->port=portnr;




            nodeName = nodename;
            ariba->setProperty("endpoints", endpoint);
      //      ariba->addBootstrapHints(bootstrap);
            ariba->setProperty("bootstrap.hints", bootstrap);
            Name spovnetName1=spovnetName;
            this->port=portnr;




    // start ariba module
    ariba->start();

    // create node and join
    node = new Node( *ariba, nodeName );

    // bind NodeListener
    bool binded=node->bind( this );
    logging_info("binded: "<<binded);
    // start node module
    node->start();

    // when initiating, you can define the overlay type, default is Chord [CHORD_OVERLAY]
//    SpoVNetProperties params;
    //params.setBaseOverlayType( SpoVNetProperties::ONE_HOP_OVERLAY ); // alternative: OneHop

    // initiate the spovnet
    logging_info("Receiver-"<<nodename<<": initiating spovnet");
    node->initiate(spovnetName1);


    // join the spovnet
    logging_info("joining spovnet "<< spovnetName1.toString());
    node->join(spovnetName1);




    logging_info( "Receiver-"<<nodename<<": hamcast starting up with"
                  << " [spovnetid " << node->getSpoVNetId().toString() << "]"
                  << " and [nodeid " << node->getNodeId().toString() << "]" );

    mcpo->joinGroup(ServiceID(1234));
    std::cout << " started1: " << spovnetName1.toString() << " as "
                << nodeName.toString()
                   <<" looking for "
                   << bootstrap << " being " << endpoint << std::endl;

}



// implementation of the startup interface
void Receiver::startup() {
    AribaModule* ariba=new AribaModule();
//   startup2(ariba,"node2-receiver","tcp{5004};"/*+ariba->getProperty("endpoints")*/,
//                                "hamcast{ip{127.0.0.1};tcp{5005}};"/*+ariba->getProperty("bootstrap.hints")*/,
//                                  "hamcast", 666);

    startup2(ariba,this->name, this->m_endpoint,this->m_bootstrap, this->m_spovnetName, this->port);

}

// implementation of the startup interface
void Receiver::shutdown() {
    ariba->stop();
    logging_info( "receiver service starting shutdown sequence ..." );


    // leave spovnet
    node->leave();

    // unbind node listener
    node->unbind( this );

    // deleting mcpo instance
    if( mcpo != NULL) delete mcpo;

    // stop the ariba module


    // delete node and ariba module
    delete node;
    delete ariba;

    // now we are completely shut down
    logging_info( "receiver service shut down" );
}


/*
 * MCPO::ReceiverInterface interface
 */
void Receiver::receiveData( const DataMessage& msg ) {



    logging_info(this->name <<": MESSAGE RECEIVED : " << msg.getMessage()->getPayload().getBuffer() );

    }

void Receiver::receiveData( const DataMessage& msg, ServiceID sid ){

    std::cout << this->name <<": CURRENT GROUP ID:" <<sid.toString() << std::endl;

    this->receiveData(msg);
}



void Receiver::serviceIsReady() {

    logging_info("serviceIsReady()");

    Timer::start();

    std::string str="Hello World!";
       std::cout << "sending message: "<< str << std::endl;
       logging_info("sending message: "<< str);
       DataMessage msg= DataMessage(str.c_str(),str.size());
       mcpo->sendToGroup(msg, group);


}


/*
 * NodeListener interface
 */
void Receiver::onJoinCompleted( const SpoVNetID& vid ) {
    logging_info( "receiver node join completed, spovnetid=" << vid.toString() );

    // initialize the MCPO service
    mcpo = new MCPO(this, port, ariba, node);
    group= ServiceID(1234);

    mcpo->joinGroup(group);




}

void Receiver::onJoinFailed( const SpoVNetID& vid ) {
    logging_error("receiver node join failed, spovnetid=" << vid.toString() );
}

void Receiver::onLeaveCompleted( const SpoVNetID& vid ){
    logging_info("receiver node leave completed, spovnetid=" << vid.toString() );
}

void Receiver::onLeaveFailed( const SpoVNetID& vid ){
    logging_error("receiver node leave failed, spovnetid=" << vid.toString() );
}



void Receiver::setCluster(unsigned int layer, NodeID leader, NodeID rp, std::vector<NodeID> members) {

}

void Receiver::setRemoteCluster(unsigned int layer, NodeID leader, std::vector<NodeID> members) {

}

void Receiver::setK(unsigned int k) {

}

void Receiver::eventFunction()
{   std::string str="Hello World!"+this->name;

    std::cout << "sending message: "<< str << std::endl;
    logging_info("sending message: "<< str);
    DataMessage msg= DataMessage(str.c_str(),str.size());
    mcpo->sendToGroup(msg, group);

}

void Receiver::join(int group)
{
    mcpo->joinGroup(ServiceID(group));

}

}} // namespace ariba, application,
