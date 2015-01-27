#include "receiver.h"

receiver::receiver(std::string group_uri, InstanceHandlers &handlers, ariba::ServiceID ariba_sid, ariba::AribaModule* ariba,ariba::Node* node)
    : m_group_uri(group_uri),
      m_handlers(handlers),
      m_ariba_sid(ariba_sid),
      m_node(node),
      m_ariba(ariba),
      ready(false)
{
  // m_group_uri.append(group_uri);
    m_mcpo= new ariba::services::mcpo::MCPO(this,m_ariba_sid,m_ariba, m_node);
    logging_info("joining group"<< m_ariba_sid.toString());
    }

    receiver::~receiver(){
        delete m_mcpo;
    }
    /* receiver interface*/
    void receiver::receiveData(const DataMessage& msg)
    {

        HC_LOG_DEBUG("GOT INTO RECEIVE_DATA");


        unsigned char* str= msg.getMessage()->getPayload().getBuffer();
        int msgsize=msg.getMessage()->getPayload().getLength()/8;
        hamcast::uri* uri_with_port=return_uri(0);
        m_handlers.recv_cb(m_handlers.m_handle, str, msgsize, uri_with_port, uri_with_port->c_str());
        HC_LOG_DEBUG("receiveData DataMessage - size: "<< msg.getSize()/8);
    }

    void receiver::receiveData(const DataMessage &msg, ServiceID sID)
    {
       // std::cout <<" GOT INTO RECEIVE_DATA"<< std::endl;
        void* str= msg.getData();

        int msgsize=(msg.getSize()/8);
       // std::cout << "Got Message from: "<< m_group_uri << std::endl;


        hamcast::uri* uri_with_port=return_uri(sID);


        m_handlers.recv_cb(m_handlers.m_handle, str, msgsize, uri_with_port, uri_with_port->c_str());
       // std::cout << "receiveData DataMessage - content: "<< msg.getMessage()->getPayload().getBuffer() <<" size: "<< msg.getSize()/8 << std::endl;

        delete uri_with_port;
    }

    hamcast::uri* receiver::return_uri(ServiceID sID){
          std::string uri="ariba://";
        uri.append(m_group_uri.c_str());
        uri.append(":");
        uri.append(sID.toString());
        hamcast::uri* uri_with_port=new hamcast::uri(uri);
        return uri_with_port;
    }

    void receiver::serviceIsReady()
    {
       HC_LOG_INFO("Service is Ready - Timer started");
        ready=true;

    }


    /**/
    void receiver::setCluster(unsigned int layer, ariba::NodeID leader, ariba::NodeID rp, std::vector<ariba::NodeID> members)
    {
    }

    void receiver::setRemoteCluster(unsigned int layer, ariba::NodeID leader, std::vector<ariba::NodeID> members)
    {
    }

    void receiver::leave(ServiceID sID)
    {
        logging_info("leaving serviceID: " << sID.toString());
       m_mcpo->leaveGroup(sID);
        std::remove(ports.begin(),ports.end(),sID);

    }

    void receiver::join(ServiceID sID)
    {
        logging_info("receiver joined group: "<< sID.toString());
        m_mcpo->joinGroup(sID);
        ports.push_back(sID);
    }


