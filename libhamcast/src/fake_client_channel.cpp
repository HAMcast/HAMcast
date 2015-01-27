/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/


#include <string>
#include <fstream>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>

#include "hamcast/exception.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/hamcast_logging.h"

#include "hamcast/util/config_map.hpp"
#include "hamcast/util/id_generator.hpp"
#include "hamcast/util/storage_semaphore.hpp"

#include "hamcast/detail/client_channel_msg.hpp"

#include "hamcast/ipc/api.hpp"
#include "hamcast/ipc/client_channel.hpp"
#include "hamcast/ipc/fake_client_channel.hpp"

using std::cout;
using std::endl;

#ifdef HC_USE_FAKE_MIDDLEWARE

#include "void.hpp"
#include "tunnel.hpp"
#include "scribe.hpp"
#include "ip_instance.hpp"
#include "scribe_instance.hpp"

namespace hamcast { namespace ipc { namespace {

// forward declarations

void recv_callback(hc_module_instance_handle_t mod_hdl, const void* data, int data_len, const hc_uri_t* uri_obj, const char* uri_str);
void event_callback(hc_module_instance_handle_t handle, int etype, const hc_uri_t* uri_obj, const char* uri_cstr);
void atmoic_message_size_callback(hc_module_instance_handle_t mod_hdl, size_t size);

struct fake_sink : util::sink
{
    void write(size_t, const void*) { }
    void flush() { }
    void close() { }
    bool closed() const { return true; }
    util::native_socket write_handle() const { return util::invalid_socket; }
};

struct fake_source : util::source
{
    bool wait_for_data(long, long) { return false; }
    size_t read_some(size_t, void*) { return 0; }
    void read(size_t, void*) { }
    void close() { }
    bool closed() const { return true; }
    util::native_socket read_handle() const { return util::invalid_socket; }
    bool has_buffered_data() const { return false; }
};

struct network_interface : ref_counted
{
    virtual void join(const uri& what) = 0;
    virtual void leave(const uri& what) = 0;
    virtual void send(const uri& whom, const void* what, int what_len, unsigned char ttl) = 0;
    virtual uri map(const uri& what) = 0;
    virtual void neighbor_set(std::vector<uri>& storage) = 0;
    virtual void group_set(std::vector<std::pair<uri, int> >& storage) = 0;
    virtual void children_set(std::vector<uri>& storage, const uri& group) = 0;
    virtual void parent_set(std::vector<uri>& storage, const uri& group) = 0;
    virtual bool designated_host(const uri& group) = 0;
    virtual void service_discovery_results(std::map<std::string, std::string>&) = 0;
    virtual interface_id id() const = 0;
};

typedef intrusive_ptr<network_interface> network_interface_ptr;

template<class Derived>
struct network_interface_mixin : network_interface
{
    std::map<uri, size_t> m_joined_groups;
    network_interface_mixin(interface_id iid) : m_id(iid) { }
    inline Derived* dthis() { return static_cast<Derived*>(this); }
    interface_id m_id;
    void join(const uri& what)
    {
        if (m_joined_groups.count(what) == 0)
        {
            m_joined_groups[what] = 1;
            dthis()->m_impl.join(what);
        }
        else {
            ++m_joined_groups[what];
        }
    }
    void leave(const uri& what)
    {
        if (m_joined_groups.count(what) > 0)
        {
            if (--m_joined_groups[what] == 0)
            {
                dthis()->m_impl.leave(what);
                m_joined_groups.erase(what);
            }
        }
    }
    uri map(const uri& what) { return dthis()->m_impl.map(what); }
    interface_id id() const { return m_id; }
    void send(const uri& whom, const void* data, int len, unsigned char ttl)
    {
        dthis()->m_impl.send(whom, data, len, ttl);
    }
    void neighbor_set(std::vector<uri>& storage)
    {
        dthis()->m_impl.neighbor_set(storage);
    }
    void group_set(std::vector<std::pair<uri, int> >& storage)
    {
        dthis()->m_impl.group_set(storage);
    }
    void children_set(std::vector<uri>& storage, const uri& group)
    {
        dthis()->m_impl.children_set(storage, group);
    }
    void parent_set(std::vector<uri>& storage, const uri& group)
    {
        dthis()->m_impl.parent_set(storage, group);
    }
    bool designated_host(const uri& group)
    {
        return dthis()->m_impl.designated_host(group);
    }
};

struct ip_interface : network_interface_mixin<ip_interface>
{
    typedef network_interface_mixin<ip_interface> super;
    ipm::ip_instance m_impl;
    ip_interface(interface_id        iid,
                 hc_log_fun_t        arg0,
                 hc_event_callback_t arg1,
                 hc_recv_callback_t  arg2,
                 hc_atomic_msg_size_callback_t arg3,
                 hc_module_instance_handle_t handle)
        : super(iid), m_impl(arg0, arg1, arg2, arg3)
    {
        m_impl.set_handle(handle);
    }
    ip_interface(interface_id            iid,
                 hc_log_fun_t            arg0,
                 hc_event_callback_t     arg1,
                 hc_recv_callback_t      arg2,
                 hc_atomic_msg_size_callback_t arg3,
                 const unsigned int      arg4,
                 const std::string&      arg5,
                 struct sockaddr_storage arg6,
                 hc_module_instance_handle_t handle)
        : super(iid), m_impl(arg0, arg1, arg2, arg3, arg4, arg5, arg6)
    {
        m_impl.set_handle(handle);
    }
    void service_discovery_results(std::map<std::string, std::string>& result)
    {
        result["if_name"] = m_impl.get_name();
        result["if_addr"] = "ip://" + m_impl.get_address();
        result["if_tech"] = "IP";
    }
};


struct tunnel_interface : network_interface_mixin<tunnel_interface>
{
    typedef network_interface_mixin<tunnel_interface> super;
    tunnel_module::tunnel m_impl;
    tunnel_interface(interface_id              iid,
                 hc_log_fun_t                  arg0,
                 hc_event_callback_t           arg1,
                 hc_recv_callback_t            arg2,
                 hc_atomic_msg_size_callback_t arg3,
                 hc_module_instance_handle_t   handle)
        : super(iid), m_impl(arg0, arg1, arg2, arg3)
    {
        m_impl.set_handle(handle);
    }
    tunnel_interface(interface_id              iid,
                 hc_log_fun_t                  arg0,
                 hc_event_callback_t           arg1,
                 hc_recv_callback_t            arg2,
                 hc_atomic_msg_size_callback_t arg3,
                 const std::string&            arg4,
                 const std::string&            arg5,
                 const std::string&            arg6,
                 const std::string&            arg7,
                 hc_module_instance_handle_t   handle)
        : super(iid), m_impl(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
    {
        m_impl.set_handle(handle);
    }
    void service_discovery_results(std::map<std::string, std::string>& result)
    {
        result["if_name"] = m_impl.get_ifname();
        //result["if_addr"] = "tun://" + m_impl.get_ifaddr();
        result["if_addr"] = "tun://192.168.1.20:1607";
        result["if_tech"] = "tunnel";
    }
};

struct scribe_interface : network_interface_mixin<scribe_interface>
{
    typedef network_interface_mixin<scribe_interface> super;
    scribe::scribe_instance m_impl;

    scribe_interface(interface_id                   iid,
                     hc_event_callback_t            arg0,
                     hc_recv_callback_t             arg1,
                     hc_atomic_msg_size_callback_t  arg2,
                     ChimeraState*                  state,
                     hc_module_instance_handle_t    handle)
        : super(iid), m_impl(arg0, arg1, arg2)
    {
        m_impl.set_state(state);
        m_impl.do_maintenance();
        m_impl.set_handle(handle);
    }

    scribe_interface(interface_id                   iid,
                     hc_event_callback_t            arg0,
                     hc_recv_callback_t             arg1,
                     hc_atomic_msg_size_callback_t  arg2,
                     bool                           arg3,
                     bool                           arg4,
                     ChimeraState*                  state,
                     hc_module_instance_handle_t    handle)
        : super(iid), m_impl(arg0, arg1, arg2, arg3, arg4)
    {
        m_impl.set_state(state);
        m_impl.do_maintenance();
        m_impl.set_handle(handle);
    }

    void service_discovery_results(std::map<std::string, std::string>& result)
    {
        result["if_name"] = m_impl.get_name();
        result["if_addr"] = "scribe://" + m_impl.get_address();
        result["if_tech"] = "ALM";
    }


};

struct void_interface : network_interface_mixin<void_interface>
{
    typedef network_interface_mixin<void_interface> super;
    void_module m_impl;
    void_interface(interface_id iid,
                   hc_event_callback_t         ecb,
                   hc_recv_callback_t          rcb,
                   hc_module_instance_handle_t handle)
        : super(iid), m_impl(ecb, rcb)
    {
        m_impl.set_handle(handle);
    }
    void service_discovery_results(std::map<std::string, std::string>& result)
    {
        result["if_name"] = "void";
        result["if_addr"] = "void://";
        result["if_tech"] = "void";
    }
};

namespace { scribe_interface* m_scribe = 0; }

void scribe_forward_handler(Key** key, Message** msg, ChimeraHost** next_host)
{
    HC_LOG_TRACE("");
    if ((*msg)->type == SCRIBE_MSG_JOIN) {
        HC_LOG_DEBUG("Forward Join ...");
        m_scribe->m_impl.forward_join (*key, *msg, *next_host);
    }
}

void scribe_deliver_handler(Key* key, Message* msg)
{
    HC_LOG_TRACE("");
    switch(msg->type) {
        case SCRIBE_MSG_CREATE:
            m_scribe->m_impl.deliver_create(key, msg);
            break;
        case SCRIBE_MSG_JOIN:
            m_scribe->m_impl.deliver_join(key, msg);
            break;
        case SCRIBE_MSG_LEAVE:
            m_scribe->m_impl.deliver_leave(key, msg);
            break;
        case SCRIBE_MSG_MULTICAST:
            m_scribe->m_impl.deliver_multicast(key, msg);
            break;
        case SCRIBE_MSG_RP_REQUEST:
            m_scribe->m_impl.deliver_rp_request(key, msg);
            break;
        case SCRIBE_MSG_RP_REPLY:
            m_scribe->m_impl.deliver_rp_reply(key, msg);
            break;
        case SCRIBE_MSG_PARENT:
            m_scribe->m_impl.deliver_parent(key, msg);
            break;
        default:
            // there are more message types, print log here
            break;
    }
}

network_interface_ptr get_ip_interface(interface_id iid,
                                       hc_module_instance_handle_t handle,
                                       const util::config_map& config)
{
    HC_LOG_TRACE("iid = " << iid);
    struct ifaddrs* ifaddrs;
    if (getifaddrs(&ifaddrs) == -1)
    {
        HC_LOG_ERROR ("getifaddrs failed");
    }
    else
    {
        const std::string& config_iface = config.get("ip", "if");
        std::string iface_ip;
        std::string iface_name;
        // FIXME: this is crap, but we need it, thus it's useful
        for (struct ifaddrs* ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa ->ifa_addr->sa_family == AF_INET) // check wheter it's IPv4
            {
                char ip_addr[NI_MAXHOST];
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            ip_addr, sizeof(ip_addr), NULL, 0, NI_NUMERICHOST);
                struct sockaddr_storage* iface_addr =
                        ((struct sockaddr_storage *)ifa->ifa_addr);
                iface_ip = ip_addr;
                iface_name = ifa->ifa_name;
                unsigned int iface_index = if_nametoindex(iface_name.c_str());
                if (   iface_addr != NULL
                    && (config_iface.empty() || config_iface == iface_name)
                    && iface_ip.find("127.0.0.1") == std::string::npos
                    && iface_ip.find("127.0.1.1") == std::string::npos)
                {
                    // we've found a valid IP interface
                    HC_LOG_DEBUG("new ip_interface ("
                                 << (iface_addr->ss_family == AF_INET
                                     ? "IPv4"
                                     : "not IPv4")
                                 << "): "
                                 << iface_name << ", idx: " << iface_index
                                 << ", IP: " << iface_ip);
                    if (ifaddrs != NULL) freeifaddrs(ifaddrs);
                    return new ip_interface(iid,
                                            hc_get_log_fun(),
                                            event_callback,
                                            recv_callback,
                                            atmoic_message_size_callback,
                                            iface_index,
                                            iface_name,
                                            *iface_addr,
                                            handle);
                }
            }
        }
    }
    HC_LOG_DEBUG("could not find ip interface, use INADDR_ANY");
    if (ifaddrs != NULL) freeifaddrs(ifaddrs);
    return new ip_interface(iid,
                            hc_get_log_fun(),
                            event_callback,
                            recv_callback,
                            atmoic_message_size_callback,
                            handle);
}

network_interface_ptr get_tunnel_interface(interface_id iid,
                                           hc_module_instance_handle_t handle,
                                           const util::config_map&)
{
    HC_LOG_TRACE("iid = " << iid);
    return new tunnel_interface(iid,
                                hc_get_log_fun(),
                                event_callback,
                                recv_callback,
                                atmoic_message_size_callback,
                                "",
                                "1607",
                                "192.168.1.21",
                                "8000",
                                handle);
}

network_interface_ptr get_void_interface(interface_id iid,
                                         hc_module_instance_handle_t handle,
                                         const util::config_map&)
{
    HC_LOG_TRACE("iid = " << iid);
    return new void_interface(iid,
                              event_callback,
                              recv_callback,
                              handle);
}

network_interface_ptr get_scribe_interface(interface_id iid,
                                       hc_module_instance_handle_t handle,
                                       const util::config_map& config)
{
    HC_LOG_TRACE("iid = " << iid);
    int bootstrap_port = SCRIBE_DEFAULT_PORT;
    std::string bootstrap_addr = config.get ("scribe", "bootstrap.addr");
    std::stringstream sstr1 (config.get ("scribe", "bootstrap.port"));
    if ( !sstr1.str().empty())
        sstr1 >> bootstrap_port;

    int local_port = SCRIBE_DEFAULT_PORT;
    std::string local_addr = config.get("scribe", "local.addr");
    std::stringstream sstr2 (config.get ("scribe", "local.port"));
    if ( !sstr2.str().empty() )
        sstr2 >> local_port;

    bool mack = (config.get("scribe", "reliable") == "true");
    bool mnt = (config.get("scribe", "maintenance") == "true");
    ChimeraState *state = NULL;

    /* if local_addr is set, then it overrides interface address
     * might be useful for NATs, i.e. set local_addr to public IP
     */
    if (local_addr.length () > 0) {
        HC_LOG_DEBUG("use local_addr");
        state = chimera_init (local_addr.c_str (), local_port);
    }
    else {
        HC_LOG_DEBUG("no local_addr");
        state = chimera_init (NULL, local_port);
    }

    if (state == NULL) {
        HC_LOG_FATAL ("Unable to initialize CHIMERA overlay!");
        return NULL;
        // FIXME: should break here or throw exception
    }
    ChimeraHost* bs_host = NULL;
    if (!bootstrap_addr.empty()) {
        bs_host = host_get (state, const_cast<char*>(bootstrap_addr.c_str()),
                            bootstrap_port);
        HC_LOG_INFO("Found Bootstrap node: " << bs_host->name <<
                      "; addr: " << ntohl(bs_host->address) <<
                      "; port: " << bs_host->port <<
                      "; hash: " << key_to_string(bs_host->key));
    } else {
        HC_LOG_DEBUG("Bootstrap node not set, init new overlay.");
    }

    // default: acking disabled; the chimera guys defined false=2, true=1
    int acking = 2;

    // enable acking for multicast messages
    if (mack)
    {
        HC_LOG_INFO("ACKs enabled for all Scribe messages.");
        acking = 1;
    }
    else
    {
        HC_LOG_INFO("ACKs disabled for all Scribe messages.");
    }

    HC_LOG_DEBUG("Register Scribe messages in CHIMERA.");
    // register scribe standard message types
    chimera_register(state, SCRIBE_MSG_UNKNOWN, acking);
    chimera_register(state, SCRIBE_MSG_CREATE, 1);
    chimera_register(state, SCRIBE_MSG_JOIN, acking);
    chimera_register(state, SCRIBE_MSG_LEAVE, acking);
    chimera_register(state, SCRIBE_MSG_MULTICAST, acking);

    // register scribe control message types
    chimera_register(state, SCRIBE_MSG_PING, acking);
    chimera_register(state, SCRIBE_MSG_HEARTBEAT, acking);
    chimera_register(state, SCRIBE_MSG_RP_REQUEST, 1);
    chimera_register(state, SCRIBE_MSG_RP_REPLY, 1);
    chimera_register(state, SCRIBE_MSG_REPLICATE, 1);
    chimera_register(state, SCRIBE_MSG_PARENT, 1);

    // register upcalls
    HC_LOG_DEBUG("Register Scibe upcalls in CHIMERA.");
    chimera_forward(state, scribe_forward_handler);
    chimera_deliver(state, scribe_deliver_handler);

    HC_LOG_DEBUG("Try to join CHIMERA overlay.");
    chimera_join(state, bs_host);

    m_scribe = new scribe_interface(iid,
                                    event_callback,
                                    recv_callback,
                                    atmoic_message_size_callback,
                                    mnt,
                                    false,
                                    state,
                                    handle);
    return m_scribe;
}

struct fake_socket_data;

typedef std::map<hamcast::ipc::stream_id, uri> out_stream_map;
typedef std::map<socket_id,  fake_socket_data> fake_socket_data_map;
typedef std::list<socket_id> socket_id_list;
typedef std::map<uri, socket_id_list> uri_listening_map;

typedef std::map<interface_id, network_interface_ptr> interface_map;

struct fake_socket_data
{
    unsigned char m_ttl;
    multicast_socket::d_ptr m_obj;
    interface_map m_ifs;
    std::map<stream_id, uri> m_streams;
    std::set<uri> m_joined_groups;
    util::id_generator<ipc::stream_id> m_stream_ids;
    async_multicast_socket::receive_callback recv_cb;

    fake_socket_data() : m_ttl(1) { }

    void set_ttl(unsigned char new_value)
    {
        m_ttl = new_value;
    }

    void add_interface(const network_interface_ptr& ptr)
    {
        m_ifs.insert(std::make_pair(ptr->id(), ptr));
    }

    void remove_iface(interface_id iid)
    {
        m_ifs.erase(iid);
    }

    void get_interfaces(std::vector<interface_id>& result)
    {
        for (interface_map::iterator i = m_ifs.begin(); i != m_ifs.end(); ++i)
            result.push_back(i->first);
    }

    void clear_ifaces()
    {
        m_ifs.clear();
    }

    const std::set<uri>& joined_groups()
    {
        return m_joined_groups;
    }

    void join(const uri& group)
    {
        if (m_joined_groups.insert(group).second)
        {
            for (interface_map::iterator i = m_ifs.begin(); i != m_ifs.end(); ++i)
            {
                (i->second)->join(group);
            }
        }
    }

    void leave(const uri& group)
    {
        if (m_joined_groups.erase(group) > 0)
        {
            for (interface_map::iterator i = m_ifs.begin(); i != m_ifs.end(); ++i)
            {
                (i->second)->leave(group);
            }
        }
    }

    stream_id add_stream(const uri& group)
    {
        HC_LOG_TRACE("group = " << group.str());
        stream_id result = m_stream_ids.next();
        m_streams.insert(std::make_pair(result, group));
        return result;
    }
};

struct fake_client_channel : client_channel
{

    typedef client_channel super;

    sync_function_dispatcher<fake_client_channel> m_dispatcher;

    interface_map m_ifs;
    fake_socket_data_map m_sockets;
    util::id_generator<socket_id> m_socket_id_generator;
    util::storage_semaphore<max_channel_queue_size, util::nonblocking_t> m_ssem;

    void on_exit(const std::string&);

    void ipc_read();

    void poll_messages(size_t num);

    fake_client_channel();

    fake_socket_data& socket(socket_id sid)
    {
        fake_socket_data_map::iterator i = m_sockets.find(sid);
        HC_REQUIRE(i != m_sockets.end());
        return i->second;
    }

    socket_id add_new_socket()
    {
        socket_id result = m_socket_id_generator.next();
        m_sockets.insert(std::make_pair(result, fake_socket_data()));
        return result;
    }

    void remove_socket(socket_id sid)
    {
        m_sockets.erase(sid);
    }

    network_interface_ptr interface(interface_id iid)
    {
        interface_map::iterator i = m_ifs.find(iid);
        HC_REQUIRE(i != m_ifs.end());
        return i->second;
    }

    void add_interface(const network_interface_ptr& ptr)
    {
        if (ptr) m_ifs.insert(std::make_pair(ptr->id(), ptr));
    }

    void recv_cb(const void* data,
                 int data_len,
                 const hc_uri_t* uri_obj,
                 const char* uri_str)
    {
        HC_LOG_TRACE("data_len = " << data_len);
        HC_REQUIRE((uri_obj || uri_str) && data && data_len > 0);
        if (!m_ssem.try_acquire(static_cast<boost::int32_t>(data_len)))
        {
            std::cout << "WARN: package dropped (internal queue full)\n";
            return;
        }
        using namespace detail;
        char* data_copy = new char[data_len];
        (void) memcpy(data_copy, data, data_len);
        uri sender;
        if (uri_obj) sender = *uri_obj;
        else sender = uri_str;
        multicast_packet mp(sender, data_len, data_copy);
        client_channel_msg *result = new incoming_mcast_packet(mp);
        push(result);
    }

};

void cb_set_is_img(fake_client_channel*, util::unit&, bool) { }

void cb_get_is_img(fake_client_channel*, bool& result) { result = false; }

void cb_cr_sock_stream(fake_client_channel* self,
                       ipc::stream_id& result,
                       socket_id sid,
                       const uri& group)
{
    HC_LOG_TRACE("sid = " << sid << ", group = " << group.str());
    HC_REQUIRE(!group.empty());
    result = self->socket(sid).add_stream(group);
    HC_LOG_DEBUG("stream_id (result) = " << result);
}

void cb_get_sock_ifs(fake_client_channel* self,
                     std::vector<interface_id>& result,
                     socket_id sid)
{
    HC_LOG_TRACE("sid = " << sid);
    self->socket(sid).get_interfaces(result);
}

void cb_designated_host(fake_client_channel* self,
                        bool& result,
                        interface_id iid,
                        const uri& group)
{
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    result = self->interface(iid)->designated_host(group);
}

struct mv_less
{
    bool operator()(const std::pair<const interface_id, network_interface_ptr>& lhs,
                    interface_id rhs)
    {
        return lhs.first < rhs;
    }
    bool operator()(interface_id lhs,
                    const std::pair<const interface_id, network_interface_ptr>& rhs)
    {
        return lhs < rhs.first;
    }
};

void cb_set_sock_ifs(fake_client_channel* self,
                     hamcast::util::unit&,
                     socket_id sid,
                     std::vector<interface_id> ifs)
{
    HC_REQUIRE(!ifs.empty());
    HC_LOG_TRACE("sid = " << sid);
    std::sort(ifs.begin(), ifs.end());
    interface_map& socket_ifs = self->socket(sid).m_ifs;
    interface_map& all_ifs = self->m_ifs;
    socket_ifs.clear();
    std::set_intersection(all_ifs.begin(), all_ifs.end(),
                          ifs.begin(), ifs.end(),
                          std::inserter(socket_ifs, socket_ifs.begin()),
                          mv_less());
}

void cb_add_sock_if (fake_client_channel* self,
                    hamcast::util::unit&,
                    socket_id sid,
                    interface_id iid)
{
    HC_LOG_TRACE("sid = " << sid << ", iid = " << iid);
    self->socket(sid).add_interface(self->interface(iid));
}

void cb_rm_sock_if (fake_client_channel* self,
                   hamcast::util::unit&,
                   socket_id sid,
                   interface_id iid)
{
    HC_LOG_TRACE("sid = " << sid << ", iid = " << iid);
    self->socket(sid).m_ifs.erase(iid);
}

void cb_neighbor_set(fake_client_channel* self,
                     std::vector<uri>& result,
                     interface_id iid)
{
    HC_LOG_TRACE("iid = " << iid);
    self->interface(iid)->neighbor_set(result);
}

void cb_children_set(fake_client_channel* self,
                     std::vector<uri>& result,
                     interface_id iid,
                     const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    self->interface(iid)->children_set(result, group);
}

template<typename K, typename V>
void set_from_map(const std::map<K,V>& container, const K& key, V& storage)
{
    typename std::map<K,V>::const_iterator i = container.find(key);
    if (i != container.end()) storage = i->second;
}

void cb_get_ifs(fake_client_channel* self,
                std::vector<hamcast::interface_property>& result)
{
    static std::string s_if_name = "if_name";
    static std::string s_if_addr = "if_addr";
    static std::string s_if_tech = "if_tech";
    HC_LOG_TRACE("");
    interface_map& ifs = self->m_ifs;
    for (interface_map::iterator i = ifs.begin(); i != ifs.end(); ++i)
    {
        hamcast::interface_property p;
        p.id = (i->second)->id();
        // default values
        p.name = "";
        p.address = "";
        p.technology = "";
        std::map<std::string, std::string> sd;
        (i->second)->service_discovery_results(sd);
        set_from_map(sd, s_if_name, p.name);
        set_from_map(sd, s_if_addr, p.address);
        set_from_map(sd, s_if_tech, p.technology);
        result.push_back(p);
    }
}

void cb_cr_sock(fake_client_channel* self, socket_id& result)
{
    HC_LOG_TRACE("");
    result = self->add_new_socket();
    self->socket(result).m_ifs = self->m_ifs;
    HC_LOG_DEBUG("result = " << result);
}

void cb_rm_sock(fake_client_channel* self, hamcast::util::unit&, socket_id sid)
{
    HC_LOG_TRACE("sid = " << sid);
    self->remove_socket(sid);
}

void cb_parent_set(fake_client_channel* self,
                   std::vector<uri>& result,
                   interface_id iid,
                   const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("iid = " << iid << ", group = " << group.str());
    self->interface(iid)->parent_set(result, group);
}

void cb_group_set(fake_client_channel* self,
                  std::vector<std::pair<uri, boost::uint32_t> >& result,
                  interface_id iid)
{
    HC_LOG_TRACE("iid = " << iid);
    std::vector<std::pair<uri, int> > tmp;
    self->interface(iid)->group_set(tmp);
    std::copy(tmp.begin(), tmp.end(), std::back_inserter(result));
}

void cb_set_ttl(fake_client_channel* self,
                hamcast::util::unit&,
                socket_id sid,
                boost::uint8_t ttl_value)
{
    HC_LOG_TRACE("sid = " << sid << ", ttl = " << ttl_value);
    self->socket(sid).set_ttl(ttl_value);
}

void cb_leave(fake_client_channel* self,
              hamcast::util::unit&,
              socket_id sid,
              const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("sid = " << sid << ", u = " << group.str());
    self->socket(sid).leave(group);
}

void cb_join(fake_client_channel* self,
             hamcast::util::unit&,
             socket_id sid,
             const uri& group)
{
    HC_REQUIRE(!group.empty());
    HC_LOG_TRACE("sid = " << sid << ", u = " << group.str());
    self->socket(sid).join(group);
}

void cb_enable_events(fake_client_channel*)
{
    HC_LOG_TRACE("");
}

void cb_disable_events(fake_client_channel*)
{
    HC_LOG_TRACE("");
}

void cb_get_atomic_msg_size(fake_client_channel*,
                            boost::uint32_t& result,
                            interface_id)
{
    HC_LOG_TRACE("");
    result = default_max_msg_size;
}

fake_client_channel::fake_client_channel()
    : super(new fake_source, new fake_sink), m_dispatcher(this)
{
    m_dispatcher.init(cb_cr_sock_stream,
                      cb_get_sock_ifs,
                      cb_designated_host,
                      cb_add_sock_if,
                      cb_set_sock_ifs,
                      cb_rm_sock_if,
                      cb_neighbor_set,
                      cb_children_set,
                      cb_get_ifs,
                      cb_cr_sock,
                      cb_rm_sock,
                      cb_parent_set,
                      cb_group_set,
                      cb_set_ttl,
                      cb_leave,
                      cb_join,
                      cb_enable_events,
                      cb_disable_events,
                      cb_get_atomic_msg_size,
                      cb_set_is_img,
                      cb_get_is_img);
    util::config_map conf;
    try { conf.read_ini("hamcast.ini"); }
    catch (...)
    {
        std::cerr << "cannot parse hamcast.ini, use defaults" << std::endl;
    }
    util::id_generator<interface_id> iid_generator;
    int if_count = 0;
    if (conf.has_group("scribe"))
    {
        add_interface(get_scribe_interface(iid_generator.next(), this, conf));
        ++if_count;
    }
    if (conf.has_group("tunnel"))
    {
        add_interface(get_tunnel_interface(iid_generator.next(), this, conf));
        ++if_count;
    }
    if (conf.has_group("void"))
    {
        add_interface(get_void_interface(iid_generator.next(), this, conf));
        ++if_count;
    }
    // enable IP, if configured or no other tech is enabled
    if (conf.has_group("ip") || if_count < 1)
    {
        add_interface(get_ip_interface(iid_generator.next(), this, conf));
    }
}

void fake_client_channel::ipc_read()
{
    HC_LOG_TRACE("");
}

void fake_client_channel::poll_messages(size_t num)
{
    using namespace detail;

    HC_LOG_TRACE("");
    client_channel_msg* e = 0;
    util::serializer s(m_sink);
    //util::id_generator<ipc::stream_id> m_stream_ids_generator;
    //util::id_generator<socket_id> m_socket_id_generator;

    for (size_t i = 0; i < num; ++i)
    {
        e = m_queue.try_pop();
        HC_REQUIRE(e != 0);
        switch(e->type)
        {

            case reg_d_ptr_t:
            {
                HC_LOG_SCOPE("reg_d_ptr_t", "");
                reg_d_ptr* rdp = dynamic_cast<reg_d_ptr*>(e);
                m_sockets[rdp->sck_id].m_obj = rdp->ptr;
                delete rdp;
            }
            break;

            case reg_recv_cb_t:
            {
                HC_LOG_SCOPE("reg_recv_cb_t", "");
                register_recv_cb_msg* rrcb = e->downcast<register_recv_cb_msg>();
                m_sockets[rrcb->sck_id].recv_cb = rrcb->fun;
                delete rrcb;
            }
            break;

            // can be send directly
            case omp_t:
            {
                HC_LOG_SCOPE("omp_t", "");
                outgoing_mcast_packet* omp = dynamic_cast<outgoing_mcast_packet*>(e);
                fake_socket_data_map::iterator i = m_sockets.find(omp->sck_id);
                if (i == m_sockets.end())
                {
                    HC_LOG_ERROR("socket " << omp->sck_id << " not found");
                }
                else
                {
                    fake_socket_data& sdata = i->second;
                    out_stream_map::iterator it = sdata.m_streams.find(omp->stm_id);
                    if (it == sdata.m_streams.end())
                    {
                        cout << "error sending multicast packet" << endl;
                        HC_LOG_ERROR("socket " << omp->sck_id << " does not have stream " << omp->stm_id);
                    }
                    else
                    {
                        interface_map& ifs = sdata.m_ifs;
                        for (interface_map::iterator j = ifs.begin(); j != ifs.end(); ++j)
                        {
                            j->second->send(it->second,
                                            (void*)omp->content,
                                            omp->content_size,
                                            sdata.m_ttl);
                        }
                        multicast_socket::release_storage(sdata.m_obj, omp->content_size); // todo: richtige Stelle?
                    }
                }
                delete omp;
            }
            break;

            case osm_t:
            {
                outgoing_synchronous_message* osm = e->downcast<outgoing_synchronous_message>();
                HC_LOG_SCOPE("osm_t", "fid = " << osm->fid);
                intrusive_ptr<util::write_buffer<> > wb(new util::write_buffer<>);
                util::serializer s(wb.get());
                m_dispatcher.invoke(osm->fid, 0, osm->buf->data(), osm->buf->size(), s);
                std::pair<boost::uint32_t, void*> buf = wb->take();
                util::deserializer d(new util::read_buffer(buf.first, buf.second, true));
                message::ptr result;
                message::deserialize(d, result);
                osm->fmsg->set(result);
                delete osm;
            }
            break;

            case reg_cb_t:
            {
                HC_LOG_SCOPE("reg_cb_t", "");
                delete e;
            }
            break;

            case imp_t:
            {
                incoming_mcast_packet* imp = e->downcast<incoming_mcast_packet>();
                HC_LOG_SCOPE("imp_t", "");
                uri from = imp->mcast_packet.from();
                m_ssem.release(imp->mcast_packet.size());
                for (fake_socket_data_map::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
                {
                    if (i->second.m_joined_groups.count(from) > 0)
                    {
                        fake_socket_data& sdata = i->second;
                        if (sdata.m_obj)
                        {
                            // this socket uses blocking receive
                            multicast_socket::async_recv(i->second.m_obj,
                                                         imp->mcast_packet);
                        }
                        else if (sdata.recv_cb)
                        {
                            multicast_packet& mp = imp->mcast_packet;
                            // this socket uses an async receive callback
                            (sdata.recv_cb)(mp.from(), mp.size(), mp.data());
                        }
                        else
                        {
                            HC_LOG_ERROR("received multicast packet for a "
                                         "socket with neither a d_ptr nor "
                                         "a callback assigned");
                        }
                    }
                }
                delete imp;
            }
            break;

            default:
            {
                cout << "[send_loop] invalid message type" << endl;
                throw std::logic_error("invalid message type");
            }

        }
    }
}

void fake_client_channel::on_exit(const std::string& err_str)
{
    cout << "[on exit] trying to kill ip_instance and tunnel, err_str: "
         << err_str << endl;
    // todo: shutdown stuff
    cout << "[on exit] bye" << endl;
}

void recv_callback(hc_module_instance_handle_t mod_hdl,
                   const void* data,
                   int data_len,
                   const hc_uri_t* uri_obj,
                   const char* uri_str)
{
    HC_REQUIRE(mod_hdl != NULL);
    reinterpret_cast<fake_client_channel*>(mod_hdl)->recv_cb(data, data_len, uri_obj, uri_str);
}

void event_callback(hc_module_instance_handle_t handle,
                    int etype,
                    const hc_uri_t* uri_obj,
                    const char* uri_cstr)
{
    HC_REQUIRE(etype >= 1 && etype <= 3 && (uri_obj || uri_cstr) && handle);
    // todo ?
}

void atmoic_message_size_callback(hc_module_instance_handle_t, size_t)
{
    // todo ?
}

} } } // namespace hamcast::ipc::<anonymous>

namespace hamcast { namespace ipc {

client_channel* create_fake_client_channel()
{
    return new fake_client_channel;
}

} }

#else // HC_USE_FAKE_MIDDLEWARE

namespace hamcast { namespace ipc {

client_channel* create_fake_client_channel()
{
    HC_LOG_FATAL("create_fake_client_channel called but "
                 "HC_USE_FAKE_MIDDLEWARE is undefined");
    return NULL;
}

} }

#endif // HC_USE_FAKE_MIDDLEWARE
