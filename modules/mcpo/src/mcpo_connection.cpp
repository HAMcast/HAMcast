#include "mcpo_connection.h"
#include "openssl/hmac.h"
#include "ariba/utility/configuration/Configuration.h"
using ariba::utility::Configuration;
using ariba::services::mcpo::MCPO;
using ariba::services::mcpo::MCPOMsg;
using hamcast::uri;

//mcpo_connection::~mcpo_connection()
//{
   // std::cout << packet_count << "packets sendet" << std::endl;

    //this->shutdown();
    //ariba::utility::StartupWrapper::shutdown(this,false);
    //ariba::utility::StartupWrapper::stopSystem();
//}

mcpo_connection::mcpo_connection(InstanceHandlers &handlers,string nodename, string bootstrap_hints, string ariba_endpoints, string spovnet_name, uint32_t port)
    :m_handlers(handlers),
     m_nodename(nodename),
      m_bootstraps(bootstrap_hints),
      m_endpoints(ariba_endpoints),
      m_spovnetName(spovnet_name),
      m_port(port),
      m_ariba(new AribaModule()),
      only1Group(false),
    m_node(NULL),
      ready(false),
      m_highest_index(0)

{
    m_group_uri = "ariba://myuri.de:1234";
    m_ariba->setProperty("endpoints",/*ariba->getProperty("endpoints")+*/ariba_endpoints);
    m_ariba->addBootstrapHints(bootstrap_hints);
    m_ariba->start();
     ariba::utility::StartupWrapper::startSystem();
   ariba::utility::StartupWrapper::startup(this,false);
    Timer::setInterval(1000);

}
 mcpo_connection::~mcpo_connection(){
     std::map < std::string, receiver* >:: iterator iterator;
     for( iterator = hosts_to_receiver.begin(); iterator != hosts_to_receiver.end(); iterator++) {
         delete iterator->second;
     }

 }

void mcpo_connection::startup(){

    //TODO : SPOVNETPROPERTIES

     m_node= new Node(*m_ariba , this->m_nodename);

    bool binded=m_node->bind(this);

    HC_LOG_DEBUG("binded "+binded);


    m_node->start();
    HC_LOG_DEBUG("node started ");

//		SpoVNetProperties params;
//	params.setBaseOverlayType( SpoVNetProperties::ONE_HOP_OVERLAY ); // alternative: OneHop

    m_node->initiate(m_spovnetName);
    HC_LOG_DEBUG("node initiated ");


    m_node->join(m_spovnetName);
    HC_LOG_DEBUG("node joined");


    HC_LOG_INFO(" mcpo: " + m_nodename.toString() +": starting up with"
                + " [spovnetid " + m_node->getSpoVNetId().toString() + "]"
                + " and [nodeid " + m_node->getNodeId().toString() + "]");

    HC_LOG_INFO("started mcpo: "+this->toString());

  //  hc_uri_t uri("myuri.de");
  // getMCPOinstance(&uri)->joinGroup(ServiceID(1234));
     std::cout << "my node id: "<< m_node->getNodeId() << std::endl;
          }

void mcpo_connection::hc_shutdown(){

    HC_LOG_TRACE("I AM IN HC_SHUTDOWN");

    ariba::utility::StartupWrapper::shutdown(this,false);
   ariba::utility::StartupWrapper::stopSystem();
}

void mcpo_connection::eventFunction()
{


}

string mcpo_connection::getNodeID()
{
   ariba::NodeID id= m_node->generateNodeId(m_nodename);
    HC_LOG_TRACE("node id: "+ id.toString());

    return id.toString();
}


//node.cpp does the same; generating it here, to avoid race conditions while loading module
string mcpo_connection::getSpovnetID()
{

   SpoVNetID id=m_spovnetName.toSpoVNetId();
   return id.toString();
}

std::vector<hamcast::uri> mcpo_connection::get_neighbors()
{

    return get_uris_from_NodeIDs(m_node->getNeighborNodes());
}

std::vector<hamcast::uri> mcpo_connection::get_children(const hc_uri_t *uri)
{

    std::vector<ariba::NodeID> result;
   receiver* recv=getReceiver(uri);

    if(recv!=NULL && recv->isReady() ){
        result=recv->getChildren();

    }else{
        return get_uris_from_NodeIDs(result);
    }
}

std::vector<hamcast::uri> mcpo_connection::get_parent(const hc_uri_t *uri)
{
    std::vector<ariba::NodeID> result;
    receiver* r=getReceiver(uri);

    if(r==NULL || !ready){
        return get_uris_from_NodeIDs(result);
    }

    ariba::NodeID nid=r->getParent();
            if(nid!=ariba::NodeID::UNSPECIFIED){
                result.push_back(nid);
            }
    return get_uris_from_NodeIDs(result);

}

std::vector<hc_uri_t> mcpo_connection::get_groups()
{
    HC_LOG_TRACE("");
    std::vector<hc_uri_t> result;
     std::map < std::string, receiver* >::iterator it;
     for(it=hosts_to_receiver.begin(); it!=hosts_to_receiver.end(); it++){

         hamcast::uri u("ariba://" + (it->first));
         HC_LOG_TRACE("found group: "<< it->first);
         result.push_back(u);
     }
     return result;

}

void mcpo_connection::shutdown() {

   Timer::stop();

   // logging_info( "receiver service starting shutdown sequence ..." );
    HC_LOG_INFO("receiver service starting shutdown sequence ...");

    // leave spovnet
    m_node->leave();
    m_node->unbind(this);

    m_node->stop();
   m_ariba->stop();
    delete m_node;



    delete m_ariba;
    // now we are completely shut down
    HC_LOG_INFO("mcpoinstance::shutdown() - receiver service shut down");

}


//node listener interface
void mcpo_connection::onJoinCompleted(const ariba::SpoVNetID &vid)
{
    HC_LOG_INFO("mcpo join completed-node:" + m_nodename.toString() +" spovnet: "+vid.getAddress());
}

void mcpo_connection::onJoinFailed(const ariba::SpoVNetID &vid)
{
    HC_LOG_INFO("mcpo node join failed, spovnetid=" + vid.toString());
  //  std::cout << "FAIL::join to spovnet "<< m_spovnetName.toString() <<" failed, try again" << std::endl;

//   m_node->join(m_spovnetName);


}

void mcpo_connection::onLeaveCompleted(const SpoVNetID &vid)
{
    HC_LOG_INFO("left group");
}

void mcpo_connection::onLeaveFailed(const SpoVNetID &vid)
{
    HC_LOG_INFO("leave failed");
}

void mcpo_connection::leave(const hc_uri_t *group_uri,const char *group_uri_str)
{

    HC_LOG_TRACE("leaving group: "<< group_uri->port_as_int());
    string host=group_uri->host();
    if(only1Group)host="myuri.de";
    std::map < std::string, receiver* >:: iterator it=hosts_to_receiver.find(group_uri->user_information_and_host());

    //check to avoid creating a new receiver just for leaving (getReceiver creates receiver)
    if(it == hosts_to_receiver.end()) {
        m_handlers.event_cb(m_handlers.m_handle, 2, group_uri,group_uri->c_str());
        return;
    }
    receiver* r=getReceiver(group_uri);

    if(r != NULL){
    int i(group_uri->port_as_int());
    ServiceID sid(i);
      r->leave(sid);
    if(r->open_ports_amount()<1){
        logging_info("will delete r");
        it=hosts_to_receiver.find(group_uri->host());
        hosts_to_receiver.erase(it);
        delete r;
    }
    HC_LOG_INFO("left Group: "<< host<<":"+group_uri->port());
    logging_info("left Group: "<< host<<":"+group_uri->port());


    }else {
        HC_LOG_INFO("no need to leave group: "<< host<<":"+group_uri->port()<< " because it wasn't joined");
        logging_info("no need to leave group: "<< host<<":"+group_uri->port()<< " because it wasn't joined");
    }

    if(hosts_to_receiver.empty()){
        ready=false;
        Timer::stop();
    }
    m_handlers.event_cb(m_handlers.m_handle, 2, group_uri,group_uri->c_str());
}


void mcpo_connection::join(const hc_uri_t *group_uri,const char *group_uri_str)
{

    HC_LOG_DEBUG("got into mcpoinstance join");

    receiver* r=getReceiver(group_uri);
    r->join(group_uri->port_as_int());
}



void mcpo_connection::sendToGroup(const void *buf, int len, const hc_uri_t *group_uri)
{

    HC_LOG_DEBUG("creating message to send to groupuri: "<< *group_uri);
    DataMessage msg(buf, len);
    HC_LOG_DEBUG("preparing mcpo...");



    if(group_uri->port_as_int()==0){
        HC_LOG_ERROR("no port given - wrong uri?");
    }else{
        receiver* r=getReceiver(group_uri);

        r->sendToGroup(msg,group_uri->port_as_int());
        HC_LOG_DEBUG("send message to group: "<<group_uri->port());


    }


}


string mcpo_connection::toString()
{

    string str= "mcpoinstance: endpoints: " + m_ariba->getLocalEndpoints()  + " bootstraps: " + m_ariba->getBootstrapHints( + /*" group_uri: " + group_uri +*/" nodename: "+ m_ariba->getName() +" spovnetname: " + m_spovnetName.toString() + " port " +m_port.toString());

    return str;
}



void mcpo_connection::set_max_msg_size(size_t max)
{
    this->m_max_message_size=max;
}

//creates one if not existing
receiver* mcpo_connection::getReceiver(const hc_uri_t *group_uri){
    HC_LOG_TRACE("searching mcpo_instance");

    string host=group_uri->user_information_and_host();
    if(only1Group) host="myuri.de";
    std::map < std::string, receiver* >:: iterator it=hosts_to_receiver.find(host);
    HC_LOG_DEBUG("found a receiver: "<< (it !=hosts_to_receiver.end()));
    if(it==hosts_to_receiver.end()){
        return create_receiver(group_uri);
    }

    receiver* m=it->second;
    if(it==hosts_to_receiver.end()){HC_LOG_DEBUG("no receiver-object found with uri - something went terribly wrong >.<: "<< host);
    return NULL; }
    return m;
}

//only helper for getReceiver
receiver* mcpo_connection::create_receiver(const hamcast::uri *group_uri){
    std::string host(group_uri->user_information_and_host());
    receiver* r=new receiver(group_uri->user_information_and_host(), m_handlers,map(group_uri->user_information_and_host()),m_ariba,m_node);
    hosts_to_receiver[host]=r;
    HC_LOG_DEBUG("created new receiver for group: "+ host+ ":"+ group_uri->port());
    return r;
}

//converts nodeids to hamcast::uri compatible adresses
std::vector<hamcast::uri> mcpo_connection::get_uris_from_NodeIDs(std::vector<ariba::NodeID> nodeIDs){
   std::vector<uri>uris;

   for(vector<ariba::NodeID>::iterator it=nodeIDs.begin();it!=nodeIDs.end(); it++ ){

   string address="ariba://"+ (*it).getAddress();
   hamcast::uri u(address);
       HC_LOG_DEBUG("found uri: "<< u.str());

       uris.push_back(u);
   }
   return uris;
}



//maps to uint32 as service id
uint32_t mcpo_connection::map(std::string str)
{
    char k='k';
    unsigned char * ch = new unsigned char[str.size() + 1];
    std::copy(str.begin(), str.end(), ch);

    unsigned char hash[255];
    unsigned int hash_l;

    unsigned char* dig=HMAC(EVP_md5(),&k,1,ch,str.size(),hash,&hash_l);
    delete ch;
    uint32_t value;
    memcpy(&value,hash,4);
    return value;

}





