#ifndef MCPO_CONNECTION_H
#define MCPO_CONNECTION_H


#include "ariba/ariba.h"
#include "ariba/utility/system/StartupInterface.h"
#include "ariba/utility/system/Timer.h"
#include "ariba/utility/logging/Logging.h"
#include "ariba/utility/system/StartupWrapper.h"
#include "mcpo/MCPO.h"
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"
#include "mcpo/messages/MCPOMsg.h"
#include "receiver.h"
#include <map>
#include <string>


class mcpo_connection:

        public ariba::utility::StartupInterface,
        public NodeListener,
       public ariba::utility::Timer


{
public:




    ~mcpo_connection();

    inline void set_handle (hc_module_instance_handle_t hdl)
           {
               this->m_handlers.m_handle = hdl;
           }

    mcpo_connection(InstanceHandlers &m_handlers, string m_nodename, string bootstrap_hints, string ariba_endpoints, string spovnet_name, uint32_t m_port);

    //startup wrapper
    virtual void startup();
    virtual void shutdown();

    //node listener
    virtual void onJoinCompleted( const SpoVNetID& vid );
    virtual void onJoinFailed( const SpoVNetID& vid );
    virtual void onLeaveCompleted( const SpoVNetID& vid );
    virtual void onLeaveFailed( const SpoVNetID& vid );



    //no interface but used by mcpomodule
    virtual void sendToGroup(const void *buf, int len, const hc_uri_t *m_group_uri);
    virtual void join(const hc_uri_t *m_group_uri, const char *group_uri_str);
    virtual void leave(const hc_uri_t *m_group_uri, const char *group_uri_str);
    virtual string toString();
   // virtual hc_uri_result_t mcpo_map(const hc_uri_t* m_group_uri, const char* uri_str);
    virtual void set_max_msg_size(size_t max);

    virtual void hc_shutdown();

    //Timer interface
    virtual void eventFunction();

    string getNodeID();
    string getSpovnetID();
    std::vector<hamcast::uri> get_neighbors();
    std::vector<hamcast::uri> get_children(const hc_uri_t *uri);
    std::vector<hamcast::uri> get_parent(const hc_uri_t *uri);
    std::vector<hamcast::uri> get_groups();

private:
    AribaModule* m_ariba;
    Node* m_node;

    std::string     m_endpoints;
    std::string     m_bootstraps;
    hamcast::uri    m_group_uri;
    Name            m_nodename;
    Name            m_spovnetName;
    ServiceID       m_port;
    InstanceHandlers m_handlers;
    std::vector<uint32_t> m_freed_indizes;
    uint32_t m_highest_index;
    std::map < std::string, receiver* > hosts_to_receiver;
    bool only1Group;
 //   ariba::services::mcpo::MCPO* getMCPOinstance(const hc_uri_t *m_group_uri);
    receiver* getReceiver(const hc_uri_t *m_group_uri);
    size_t m_max_message_size;
    bool ready;
    vector<hamcast::uri>get_uris_from_NodeIDs(std::vector<ariba::NodeID> nodeIDs);
    uint32_t map(std::string str);
    receiver* create_receiver(const hamcast::uri *group_uri);

};

#endif // MCPOINSTANCE_H
