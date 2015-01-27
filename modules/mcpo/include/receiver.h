#ifndef RECEIVER_H
#define RECEIVER_H
#include "instance_handlers.h"
#include "mcpo/MCPO.h"
#include "hamcast/hamcast.hpp"

using ariba::services::mcpo::MCPO;
using namespace ariba;
class receiver:
            public ariba::services::mcpo::MCPO::ReceiverInterface
{
public:
    receiver(std::string group_uri, InstanceHandlers &handlers, ariba::ServiceID ariba_sid, ariba::AribaModule* ariba,ariba::Node* node);
    ~receiver();
    // receiver interface
    virtual void receiveData( const DataMessage& msg );
    virtual void receiveData( const DataMessage& msg , ServiceID sID);
    virtual void serviceIsReady();

    virtual void setCluster(unsigned int layer, ariba::NodeID leader, ariba::NodeID rp, std::vector<ariba::NodeID> members);
    virtual void setRemoteCluster(unsigned int layer, ariba::NodeID leader, std::vector<ariba::NodeID> members);



    //stuff
     void leave(ariba::ServiceID sID);

    void join(ariba::ServiceID sID);

    inline void sendToGroup(DataMessage message, ariba::ServiceID port)
    {
        m_mcpo->sendToGroup(message,port);
    }

    inline std::vector<ariba::NodeID> getChildren(){
    return m_mcpo->getChildren();
    }

    inline ariba::NodeID getParent(){
    return m_mcpo->getParent();
    }

    inline bool isReady(){
        return ready;
    }

    inline size_t open_ports_amount(){
        return ports.size();
    }

protected:
    MCPO* m_mcpo;
    std::string m_group_uri;
    InstanceHandlers m_handlers;
    ariba::ServiceID m_ariba_sid;
    ariba::AribaModule* m_ariba;
    ariba::Node* m_node;
    bool ready;
    vector<ServiceID>ports;

    hamcast::uri* return_uri(ServiceID sID);

};

#endif // RECEIVER_H
