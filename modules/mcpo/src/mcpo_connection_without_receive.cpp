#include "mcpo_connection.h"

using ariba::services::mcpo::MCPO;
using ariba::services::mcpo::MCPOMsg;
using hamcast::uri;

mcpo_connection::mcpo_connection(InstanceHandlers &handlers,string nodename, string bootstrap_hints, string ariba_endpoints, string spovnet_name, uint32_t port)
    :m_handlers(handlers),
     m_nodename(nodename),
      m_bootstraps(bootstrap_hints),
      m_endpoints(ariba_endpoints),
      m_spovnetName(spovnet_name),
      m_port(port),
      m_ariba(new AribaModule()),
      only1Group(true),
    m_node(NULL),
      ready(false)

{
    m_group_uri = "ariba://myuri.de:1234";
    m_ariba->setProperty("endpoints",/*ariba->getProperty("endpoints")+*/ariba_endpoints);
    m_ariba->setProperty("bootstrap.hints",/* ariba->getProperty("bootstrap")+*/bootstrap_hints);

    m_ariba->start();
     ariba::utility::StartupWrapper::startSystem();
   ariba::utility::StartupWrapper::startup(this,false);
    Timer::setInterval(1000);
      //if(Timer.isRunning()){
  //  startup();
 /*   node(ariba, nodename);
    mcpo(port, ariba, node, false);*/

}

void mcpo_connection::startup(){

    //TODO : SPOVNETPROPERTIES

     m_node= new Node(*m_ariba , this->m_nodename);

    bool binded=m_node->bind(this);

    HC_LOG_DEBUG("binded "+binded);


    m_node->start();
    HC_LOG_DEBUG("node started ");

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
          }

void mcpo_connection::hc_shutdown(){

    HC_LOG_TRACE("I AM IN HC_SHUTDOWN");


    //this->shutdown();
    ariba::utility::StartupWrapper::shutdown(this,false);
    ariba::utility::StartupWrapper::stopSystem();
}

void mcpo_connection::eventFunction()
{

 /*   std::string str="Hello World!";
       std::cout << "sending message: "<< str << std::endl;
       DataMessage msg= DataMessage(str.c_str(),str.size());
       hc_uri_t uri("myuri.de");
       getMCPOinstance(&uri)->sendToGroup(msg, ariba::ServiceID(1234));
    std::cout << "TIMER USED EVENTFUNCTION"<< std::endl;
    HC_LOG_TRACE("timer used eventfunction");*/
}

string mcpo_connection::getNodeID()
{
   NodeID id= m_node->generateNodeId(m_nodename);
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

    std::vector<NodeID> result;
    ariba::services::mcpo::MCPO* m=getMCPOinstance(uri);

    if(m==NULL || !ready ){
        return get_uris_from_NodeIDs(result);
    }

    result=m->getChildren();



    HC_LOG_DEBUG("amount of children: "<<result.size());

    std::vector<hamcast::uri> r=get_uris_from_NodeIDs(result);
    HC_LOG_DEBUG("size of r-list: "<< r.size());
    return r;
}

std::vector<hamcast::uri> mcpo_connection::get_parent(const hc_uri_t *uri)
{
    std::vector<NodeID> result;
    MCPO* m=getMCPOinstance(uri);
    if(m==NULL || !ready){
        return get_uris_from_NodeIDs(result);
    }

    NodeID nid=m->getParent();
            if(nid!=NodeID::UNSPECIFIED){
                result.push_back(nid);
            }
    return get_uris_from_NodeIDs(result);

}

std::vector<hc_uri_t> mcpo_connection::get_groups()
{
    HC_LOG_TRACE("");
    std::vector<hc_uri_t> result;
     std::map < std::string, ariba::services::mcpo::MCPO* >::iterator it;
     for(it=hosts_to_mcpo.begin(); it!=hosts_to_mcpo.end(); it++){

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
    // unbind node listener

    //node->unbind( reinterpret_cast<CommunicationListener*>(this->mcpo),  ServiceID(666) );
    m_node->stop();

    //mcpo->stop();

    //delete mcpo;

   // delete node
   m_ariba->stop();
    delete m_node;



    delete m_ariba;
    // now we are completely shut down
 //   logging_info("receiver service shut down"  );
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
}

void mcpo_connection::onLeaveCompleted(const SpoVNetID &vid)
{
    HC_LOG_INFO("left group");
}

void mcpo_connection::onLeaveFailed(const SpoVNetID &vid)
{
    HC_LOG_INFO("leave failed");
}


/* receiver interface*/
void mcpo_connection::receiveData(const DataMessage& msg)
{

    HC_LOG_DEBUG("GOT INTO RECEIVE_DATA");


    unsigned char* str= msg.getMessage()->getPayload().getBuffer();

    int msgsize=msg.getMessage()->getPayload().getLength()/8;


    //hamcast::uri *group_uri = new hamcast::uri(m_group_uri->str());
   // m_handlers.recv_cb(m_handlers.m_handle, str, msgsize, &m_group_uri, m_group_uri.c_str());




    HC_LOG_DEBUG("receiveData DataMessage - size: "<< msg.getSize()/8);
}

void mcpo_connection::receiveData(const DataMessage &msg, ServiceID sID)
{
    HC_LOG_DEBUG("GOT INTO RECEIVE_DATA");

    void* str= msg.getData();
    int msgsize=(msg.getSize()/8);
    HC_LOG_DEBUG("Got Message from: "<< m_group_uri);
   // m_handlers.recv_cb(m_handlers.m_handle, str, msgsize, &m_group_uri, m_group_uri.c_str());

    HC_LOG_DEBUG("receiveData DataMessage - content: "<< msg.getMessage()->getPayload().getBuffer() <<" size: "<< msg.getSize()/8);
}


void mcpo_connection::sendToGroup(const void *buf, int len, const hc_uri_t *group_uri)
{

    HC_LOG_DEBUG("creating message to send to groupuri: "<< *group_uri);
    DataMessage msg(buf, len);
    HC_LOG_DEBUG("preparing mcpo...");


    ariba::services::mcpo::MCPO* m=getMCPOinstance(group_uri);
    if(group_uri->port_as_int()==0){
        HC_LOG_ERROR("no port given - wrong uri?");
    }else{
        m->sendToGroup(msg, ServiceID(group_uri->port_as_int()));
        HC_LOG_DEBUG("send message to group: "<<group_uri->port());
    }


}


void mcpo_connection::serviceIsReady()
{
    Timer::start();
     HC_LOG_INFO("Service is Ready - Timer started");
    ready=true;

}


/**/
void mcpo_connection::setCluster(unsigned int layer, ariba::NodeID leader, ariba::NodeID rp, std::vector<ariba::NodeID> members)
{
}

void mcpo_connection::setRemoteCluster(unsigned int layer, ariba::NodeID leader, std::vector<ariba::NodeID> members)
{
}

void mcpo_connection::join(const hc_uri_t *group_uri,const char *group_uri_str)
{

    HC_LOG_DEBUG("got into mcpoinstance join");

    string host=group_uri->host();
    ariba::services::mcpo::MCPO* m=getMCPOinstance(group_uri);

    m->joinGroup(ServiceID(group_uri->port_as_int()));

}



void mcpo_connection::leave(const hc_uri_t *group_uri,const char *group_uri_str)
{
    HC_LOG_TRACE("leaving group: "<< group_uri->port_as_int());
    string host=group_uri->host();
    if(only1Group)host="myuri.de";
    ariba::services::mcpo::MCPO* mcpo=getMCPOinstance(group_uri);

    int i(group_uri->port_as_int());
    ServiceID sid(i);
    mcpo->leaveGroup(sid);

    HC_LOG_INFO("left Group: "<< host<<":"+group_uri->port());
    if(hosts_to_mcpo.empty()){
        ready=false;
        Timer::stop();
    }
}

string mcpo_connection::toString()
{

    string str= "mcpoinstance: endpoints: " + m_ariba->getLocalEndpoints()  + " bootstraps: " + m_ariba->getBootstrapHints( + /*" group_uri: " + group_uri +*/" nodename: "+ m_ariba->getName() +" spovnetname: " + m_spovnetName.toString() + " port " +m_port.toString());

    return str;
}

hc_uri_result_t mcpo_connection::mcpo_map(const hc_uri_t* group_uri, const char* uri_str){



    hc_uri_result_t result;

    std::string fake_uri = "ariba://myuri.de:" + group_uri->port();

    result.uri_obj= new uri(fake_uri);
    result.uri_str=NULL;

   return result;

}

void mcpo_connection::set_max_msg_size(size_t max)
{
    this->m_max_message_size=max;
}

ariba::services::mcpo::MCPO* mcpo_connection::getMCPOinstance(const hc_uri_t *group_uri){
    HC_LOG_TRACE("searching mcpo_instance");

    string host=group_uri->host();
    if(only1Group) host="myuri.de";
    std::map < std::string, ariba::services::mcpo::MCPO* >:: iterator it=hosts_to_mcpo.find(host);
    HC_LOG_DEBUG("found a mcpoinstance: "<< (it !=hosts_to_mcpo.end()));
    if(it==hosts_to_mcpo.end()){
        hosts_to_mcpo[host]=new ariba::services::mcpo::MCPO(this,this->m_port, m_ariba, this->m_node);
        HC_LOG_DEBUG("created new mcpo for group: "+ host+ ":"+ group_uri->port());
    }

    it = hosts_to_mcpo.find(std::string(host));

    ariba::services::mcpo::MCPO* m=it->second;
    if(it==hosts_to_mcpo.end()){HC_LOG_DEBUG("no mcpo-object found with uri: "<< host);
    return NULL; }
    return m;
}

std::vector<hamcast::uri> mcpo_connection::get_uris_from_NodeIDs(std::vector<NodeID> nodeIDs){
   std::vector<uri>uris;

   for(vector<NodeID>::iterator it=nodeIDs.begin();it!=nodeIDs.end(); it++ ){

   string address="ariba://"+ (*it).getAddress();
   hamcast::uri u(address);
       HC_LOG_DEBUG("found uri: "<< u.str());

       uris.push_back(u);
   }
   return uris;
}



