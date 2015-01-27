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

#ifndef HAMCAST_IPC_API_HPP
#define HAMCAST_IPC_API_HPP

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/socket_id.hpp"
#include "hamcast/util/unit.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/ipc/function_id.hpp"
#include "hamcast/membership_event.hpp"
#include "hamcast/ipc/sync_function.hpp"
#include "hamcast/interface_property.hpp"

// HC_DOCUMENTATION is predefined by doxygen.
// This block "hides" the real implementation from the documentation.
#ifdef HC_DOCUMENTATION
namespace hamcast { namespace ipc {

/**
 * @addtogroup IPC
 * @{
 */

/**
 * @brief Create a new socket and return the id of the newly created socket.
 * @returns A new multicast socket id.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::multicast_socket}.
 */
socket_id create_socket() { }

/**
 * @brief Delete a socket (this leaves all groups).
 * @param sid The ID of the socket that should be deleted.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::~multicast_socket}.
 */
void delete_socket(socket_id sid) { }

/**
 * @brief Create a new output stream.
 * @param sid The outgoing socket.
 * @param group The outgoing multicast group.
 * @returns A new multicast stream id, that's assigned to @p sid.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::send} implicitly.
 */
stream_id create_send_stream(socket_id sid, const uri& group);

/**
 * @brief Join a multicast group.
 * @param sid The ID of the socket.
 * @param group The multicast group you want to join.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::join}.
 */
void join(socket_id sid, const uri& group) { }

/**
 * @brief Leave a multicast group.
 * @param sid The ID of the socket.
 * @param group The multicast group you want to leave.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::leave}.
 */
void leave(socket_id sid, const uri& group) { }

/**
 * @brief Set the TTL value for a socket.
 * @param sid The ID of the socket.
 * @param value The new maximum hop count value.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::set_ttl}.
 */
void set_ttl(socket_id sid, boost::uint8_t value) { }

/**
 * @brief Get all interfaces that are assigned to a given socket.
 * @param sid The ID of a socket.
 * @returns A vector of all interfaces that are assigned
 *         to @p sid (might be empty).
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::interfaces()}.
 */
std::vector<interface_id> get_sock_interfaces(socket_id sid) { }

/**
 * @brief Force the socket to use the given interface.
 * @param sid The socket ID.
 * @param iid The ID of the interface you want to add.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::add_interface}.
 */
void add_sock_interface(socket_id sid, interface_id iid) { }

/**
 * @brief Remove the given interface from a socket.
 * @param sid The socket ID.
 * @param iid The ID of the interface you want to remove.
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::del_interface}.
 */
void del_sock_interface(socket_id sid, interface_id iid) { }

/**
 * @brief Force the socket to use only the given interfaces.
 * @param sid The socket ID.
 * @param ifs A vector of (valid) interface IDs.
 * @pre <code>!ifs.empty()</code>
 * @warning You should <b>not</b> call this functor manually. It is used
 *          by {@link multicast_socket::set_interfaces}.
 */
void set_sock_interfaces(socket_id sid, const std::vector<interface_id>& ifs) {}

/**
 * @brief Get all known multicast interfaces.
 * @returns All known interfaces (meta informations and ID).
 */
std::vector<interface_property> get_interfaces() { }

/**
 * @brief Returns all registered multicast groups on a given interface.
 *
 * All groups are given as a pair of an uint32_t and an uri.
 * The uint32_t is 0 if the uri is registered in listener state,
 * 1 if the uri is registered in sender state and 2 if it's registered
 * in both sender and listener state.
 *
 * @param iid The interface ID.
 * @returns All registered (and known) multicast groups on the interface @p iid.
 */
std::vector<std::pair<uri, boost::uint32_t> > group_set(interface_id iid) { }

/**
 * @brief Get all known multicast routing neighbors on a given interface.
 * @param iid The interface ID.
 * @returns All registered multicast groups on the interface @p iid.
 */
std::vector<uri> neighbor_set(interface_id iid) { }

/**
 * @brief Get a set of child nodes that receive multicast data from a
 *        specified interface for a given group.
 * @param iid The (parent) interface ID.
 * @param group The multicast group.
 * @returns A list of addresses, encoded as URIs.
 */
std::vector<uri> children_set(interface_id iid, const uri& group) { }

/**
 * @brief Get a set of neighbors from which the current node receives
 *        multicast data at a given interface for the specified group.
 * @param iid The interface ID.
 * @param group The multicast group.
 * @returns A list of addresses, encoded as URIs.
 */
std::vector<uri> parent_set(interface_id iid, const uri& group) { }

/**
 * @brief Inquire whetever the host has the role of a designated forwarder
 *        resp. querier, or not.
 * @param iid The interface ID.
 * @param group The multicast group.
 * @returns @c true if the host has the role of a designated forwarder
 *         for the multicast group @p group on the interface @p iid;
 *         otherwise @c false.
 */
bool designated_host(interface_id iid, const uri& group) { }

/**
 * @brief Enable this application to receive membership events.
 */
void enable_events();

/**
 * @brief Disable membership events for this application.
 */
void disable_events();

bool is_img();

void is_img(bool);

/**
 * @brief Get the current atomic message size of @p iid.
 * @param iid The interface ID.
 * @returns Current atomic message size.
 */
boost::uint32_t get_atomic_msg_size(interface_id iid) { }

/**
 * @}
 */

} } // namespace hamcast::ipc

// "real" implementation as (extern declared) functors
#else
namespace hamcast { namespace ipc {

typedef sync_function<fid_set_is_img_flag, util::unit, bool>
        set_is_img_flag_t;
extern set_is_img_flag_t set_is_img_flag;

typedef sync_function<fid_get_is_img_flag, bool>
        get_is_img_flag_t;
extern get_is_img_flag_t get_is_img_flag;

inline bool is_img() { return get_is_img_flag(); }
inline void is_img(bool value) { set_is_img_flag(value); }

typedef sync_function<fid_create_socket, socket_id>
        create_socket_t;
extern create_socket_t create_socket;

typedef sync_function<fid_delete_socket, util::unit, socket_id>
        delete_socket_t;
extern delete_socket_t delete_socket;

typedef sync_function<fid_create_send_stream, stream_id, socket_id, uri>
        create_send_stream_t;
extern create_send_stream_t create_send_stream;

typedef sync_function<fid_join, util::unit, socket_id, uri>
        join_t;
extern join_t join;

typedef sync_function<fid_leave, util::unit, socket_id, uri>
        leave_t;
extern leave_t leave;

typedef sync_function<fid_set_ttl, util::unit, socket_id, boost::uint8_t>
        set_ttl_t;
extern set_ttl_t set_ttl;

typedef sync_function<fid_get_sock_interfaces, std::vector<interface_id>, socket_id>
        get_sock_interfaces_t;
extern get_sock_interfaces_t get_sock_interfaces;

typedef sync_function<fid_add_sock_interface, util::unit, socket_id, interface_id>
        add_sock_interface_t;
extern add_sock_interface_t add_sock_interface;

typedef sync_function<fid_del_sock_interface, util::unit, socket_id, interface_id>
        del_sock_interface_t;
extern del_sock_interface_t del_sock_interface;

typedef sync_function<fid_get_interfaces, std::vector<interface_property> >
        get_interfaces_t;
extern get_interfaces_t get_interfaces;

typedef sync_function<fid_group_set, std::vector<std::pair<uri, boost::uint32_t> >, interface_id>
        group_set_t;
extern group_set_t group_set;

typedef sync_function<fid_neighbor_set, std::vector<uri>, interface_id>
        neighbor_set_t;
extern neighbor_set_t neighbor_set;

typedef sync_function<fid_children_set, std::vector<uri>, interface_id, uri>
        children_set_t;
extern children_set_t children_set;

typedef sync_function<fid_parent_set, std::vector<uri>, interface_id, uri>
        parent_set_t;
extern parent_set_t parent_set;

typedef sync_function<fid_designated_host, bool, interface_id, uri>
        designated_host_t;
extern designated_host_t designated_host;

typedef sync_function<fid_set_sock_interfaces, util::unit, socket_id,
                      std::vector<interface_id> >
        set_sock_interfaces_t;
extern set_sock_interfaces_t set_sock_interfaces;

typedef sync_function<fid_enable_events, util::unit> enable_events_t;
extern enable_events_t enable_events;

typedef sync_function<fid_enable_events, util::unit> disable_events_t;
extern disable_events_t disable_events;

typedef sync_function<fid_get_atomic_msg_size, boost::uint32_t, interface_id>
        get_atomic_msg_size_t;
extern get_atomic_msg_size_t get_atomic_msg_size;

struct sync_function_reply_args
{
    request_id rid;
    const void* content;
    boost::uint32_t content_size;
    util::serializer* ipc_out;
};

template<typename Fun, typename Callback>
inline void sync_fun_reply(sync_function_reply_args& args, Fun& fun, Callback& cb)
{
    fun.reply(args.rid, args.content, args.content_size, *(args.ipc_out), cb);
}

template<typename Client>
class sync_function_dispatcher
{

 public:

    sync_function_dispatcher(Client* c) : m_ptr(c) { }

    template<typename CrSockStream,
             typename GetSockIfs,
             typename DesignatedHost,
             typename AddSockIf,
             typename SetSockIfs,
             typename RmSockIf,
             typename NeighborSet,
             typename ChildrenSet,
             typename GetIfs,
             typename CrSock,
             typename RmSock,
             typename ParentSet,
             typename GroupSet,
             typename SetTtl,
             typename Leave,
             typename Join,
             typename EnableEvents,
             typename DisableEvents,
             typename GetAtomicMsgSize,
             typename SetIsImg,
             typename GetIsImg>
    void init(CrSockStream cr_sock_stream_fun,
              GetSockIfs get_sock_ifs_fun,
              DesignatedHost designated_host_fun,
              AddSockIf add_sock_if_fun,
              SetSockIfs set_sock_ifs_fun,
              RmSockIf rm_sock_if_fun,
              NeighborSet neighbor_set_fun,
              ChildrenSet children_set_fun,
              GetIfs get_ifs_fun,
              CrSock cr_sock_fun,
              RmSock rm_sock_fun,
              ParentSet parent_set_fun,
              GroupSet group_set_fun,
              SetTtl set_ttl_fun,
              Leave leave_fun,
              Join join_fun,
              EnableEvents enable_events_fun,
              DisableEvents disable_events_fun,
              GetAtomicMsgSize atomic_msg_size_fun,
              SetIsImg set_is_img_fun,
              GetIsImg get_is_img_fun)
    {
        using boost::bind;
        m_cr_sock_stream      = bind(cr_sock_stream_fun,  m_ptr, _1, _2, _3);
        m_get_sock_ifs        = bind(get_sock_ifs_fun,    m_ptr, _1, _2);
        m_designated_host     = bind(designated_host_fun, m_ptr, _1, _2, _3);
        m_add_sock_if         = bind(add_sock_if_fun,     m_ptr, _1, _2, _3);
        m_set_sock_ifs        = bind(set_sock_ifs_fun,    m_ptr, _1, _2, _3);
        m_rm_sock_if          = bind(rm_sock_if_fun,      m_ptr, _1, _2, _3);
        m_neighbor_set        = bind(neighbor_set_fun,    m_ptr, _1, _2);
        m_children_set        = bind(children_set_fun,    m_ptr, _1, _2, _3);
        m_get_ifs             = bind(get_ifs_fun,         m_ptr, _1);
        m_cr_sock             = bind(cr_sock_fun,         m_ptr, _1);
        m_rm_sock             = bind(rm_sock_fun,         m_ptr, _1, _2);
        m_parent_set          = bind(parent_set_fun,      m_ptr, _1, _2, _3);
        m_group_set           = bind(group_set_fun,       m_ptr, _1, _2);
        m_set_ttl             = bind(set_ttl_fun,         m_ptr, _1, _2, _3);
        m_leave               = bind(leave_fun,           m_ptr, _1, _2, _3);
        m_join                = bind(join_fun,            m_ptr, _1, _2, _3);
        m_enable_events       = bind(enable_events_fun,   m_ptr);
        m_disable_events      = bind(disable_events_fun,  m_ptr);
        m_get_atomic_msg_size = bind(atomic_msg_size_fun, m_ptr, _1, _2);
        m_set_is_img_flag     = bind(set_is_img_fun,      m_ptr, _1, _2);
        m_get_is_img_flag     = bind(get_is_img_fun,      m_ptr, _1);
    }

    void invoke(function_id fid, request_id rid,
                const void* content, boost::uint32_t content_size,
                util::serializer& ipc_out)
    {
        sync_function_reply_args args = {rid, content, content_size, &ipc_out};
        switch (fid)
        {

          case fid_get_is_img_flag:
            sync_fun_reply(args, get_is_img_flag, m_get_is_img_flag);
            break;

         case fid_set_is_img_flag:
            sync_fun_reply(args, set_is_img_flag, m_set_is_img_flag);
            break;

         case fid_create_socket:
            sync_fun_reply(args, create_socket, m_cr_sock);
            break;

         case fid_delete_socket:
            sync_fun_reply(args, delete_socket, m_rm_sock);
            break;

         case fid_create_send_stream:
            sync_fun_reply(args, create_send_stream, m_cr_sock_stream);
            break;

         case fid_join:
            sync_fun_reply(args, join, m_join);
            break;

         case fid_leave:
            sync_fun_reply(args, leave, m_leave);
            break;

         case fid_set_ttl:
            sync_fun_reply(args, set_ttl, m_set_ttl);
            break;

         case fid_get_sock_interfaces:
            sync_fun_reply(args, get_sock_interfaces, m_get_sock_ifs);
            break;

         case fid_add_sock_interface:
            sync_fun_reply(args, add_sock_interface, m_add_sock_if);
            break;

         case fid_del_sock_interface:
            sync_fun_reply(args, del_sock_interface, m_rm_sock_if);
            break;

         case fid_set_sock_interfaces:
            sync_fun_reply(args, set_sock_interfaces, m_set_sock_ifs);
            break;

         case fid_get_interfaces:
            sync_fun_reply(args, get_interfaces, m_get_ifs);
            break;

         case fid_group_set:
            sync_fun_reply(args, group_set, m_group_set);
            break;

         case fid_neighbor_set:
            sync_fun_reply(args, neighbor_set, m_neighbor_set);
            break;

         case fid_children_set:
            sync_fun_reply(args, children_set, m_children_set);
            break;

         case fid_parent_set:
            sync_fun_reply(args, parent_set, m_parent_set);
            break;

         case fid_designated_host:
            sync_fun_reply(args, designated_host, m_designated_host);
            break;

         case fid_enable_events:
            sync_fun_reply(args, enable_events, m_enable_events);
            break;

         case fid_disable_events:
            sync_fun_reply(args, disable_events, m_disable_events);
            break;

         case fid_get_atomic_msg_size:
            sync_fun_reply(args, get_atomic_msg_size, m_get_atomic_msg_size);
            break;

         default: throw requirement_failed("Invalid function ID");

        }
    }

    void invoke(message::ptr& msg, util::serializer& ipc_out)
    {
        // check arguments
        HC_REQUIRE(msg && msg->type() == sync_request);
        sync_request_view srm(*msg);
        invoke(srm.fun_id(), srm.req_id(),
               msg->content(), msg->content_size(),
               ipc_out);
    }

 private:

    Client* m_ptr;
    get_is_img_flag_t::callback m_get_is_img_flag;
    set_is_img_flag_t::callback m_set_is_img_flag;
    create_send_stream_t::callback m_cr_sock_stream;
    get_sock_interfaces_t::callback m_get_sock_ifs;
    designated_host_t::callback m_designated_host;
    add_sock_interface_t::callback m_add_sock_if;
    del_sock_interface_t::callback m_rm_sock_if;
    set_sock_interfaces_t::callback m_set_sock_ifs;
    neighbor_set_t::callback m_neighbor_set;
    children_set_t::callback m_children_set;
    get_interfaces_t::callback m_get_ifs;
    create_socket_t::callback m_cr_sock;
    delete_socket_t::callback m_rm_sock;
    parent_set_t::callback m_parent_set;
    group_set_t::callback m_group_set;
    set_ttl_t::callback m_set_ttl;
    leave_t::callback m_leave;
    join_t::callback m_join;
    enable_events_t::callback m_enable_events;
    disable_events_t::callback m_disable_events;
    get_atomic_msg_size_t::callback m_get_atomic_msg_size;

};

} } // namespace hamcast::ipc

#endif

#endif // HAMCAST_IPC_API_HPP
