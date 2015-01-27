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

#include <set>
#include <dlfcn.h>
#include <iostream>
#include <algorithm>
#include <boost/thread.hpp>

#include "hamcast/hamcast_logging.h"
#include "hamcast/util/atomic_operations.hpp"

#include "session.hpp"
#include "tech_module.hpp"
#include "rw_spinlock.hpp"

namespace hamcast { namespace middleware {

void mh_release::operator()(hc_module_handle* mh)
{
    if (mh)
    {
        boost::int32_t rc = hamcast::util::add_and_fetch(&(mh->ref_count), -1);
        if (rc == 0)
        {
            mh->shutdown_fun();
            dlclose(mh->library_handle);
            free(mh);
        }
    }
}

namespace {

typedef boost::shared_lock<rw_spinlock> read_guard;
typedef boost::lock_guard<rw_spinlock> write_guard;

typedef std::map<interface_id, tech_module::ptr> instances_by_id;
typedef std::set<session_ptr> event_subscribers;

// guards both s_by_id
rw_spinlock s_instances_mtx;
// stores all instances with their interface id as key
instances_by_id s_by_id;

// guards s_event_subscribers;
rw_spinlock s_event_subscribers_mtx;
// stores all membership event subscribers
event_subscribers s_event_subscribers;

void add_instance(tech_module::ptr obj)
{
    write_guard guard(s_instances_mtx);
    s_by_id.insert(instances_by_id::value_type(obj->iface_id(), obj));
}

interface_id s_next_id = 0;

interface_id next_iid()
{
    return util::add_and_fetch(&s_next_id, 1);
}

void add_error_msg(std::string& s, bool condition, const std::string& msg)
{
    if (condition)
    {
        if (!s.empty()) s += "; ";
        s += msg;
    }
}

void check4error(void* handle, const char* error_string,
                 bool close_on_error = true)
{
    if (error_string)
    {
        std::string errstr = error_string;
        if (close_on_error) dlclose(handle);
        throw could_not_load_module(errstr);
    }
}

} // namespace <anonymous>

namespace detail {

struct tm_mapping_helper
{
    tech_module& tm;
    tm_mapping_helper(tech_module& mtm) : tm(mtm) { }
    uri operator()(const uri& what)
    {
        uri result;
        hc_uri_result_t hc_res = tm.m_handle->map_fun(tm.mod_ptr(),
                                                      &what, what.c_str());
        if (hc_res.uri_str)
        {
            result = hc_res.uri_str;
            free(hc_res.uri_str);
            if (hc_res.uri_obj) delete hc_res.uri_obj;
        }
        else if (hc_res.uri_obj)
        {
            result = *(hc_res.uri_obj);
            delete hc_res.uri_obj;
        }
        else
        {
            HC_LOG_FATAL("Module " << tm.iface_id() << " was unable to map "
                         << what.str());
        }
        return result;
    }
};

} // namespace detail

void subscribe_events(const session_ptr& subscriber)
{
    write_guard guard(s_event_subscribers_mtx);
    s_event_subscribers.insert(subscriber);
}

void unsubscribe_events(const session_ptr& subscriber)
{
    write_guard guard(s_event_subscribers_mtx);
    s_event_subscribers.erase(subscriber);
}

void deliver_event(const membership_event& event)
{
    read_guard guard(s_event_subscribers_mtx);
    for (event_subscribers::const_iterator i = s_event_subscribers.begin();
         i != s_event_subscribers.end();
         ++i)
    {
        const_cast<session&>(*(*i)).handle_event(event);
    }
}

could_not_load_module::could_not_load_module(const std::string &reason)
    : std::runtime_error(reason)
{
}

tech_module::~tech_module()
{
}

std::vector<tech_module::ptr> tech_module::instances()
{
    read_guard guard(s_instances_mtx);
    std::vector<tech_module::ptr> result;
    result.reserve(s_by_id.size());
    for (instances_by_id::const_iterator i(s_by_id.begin());
         i != s_by_id.end();
         ++i)
    {
        result.push_back(i->second);
    }
    return result;
}

void tech_module::recv_callback(hc_module_instance_handle_t mod_hdl,
                                const void* data,
                                int data_len,
                                const hc_uri_t* uri_obj,
                                const char* uri_str)
{
    HC_LOG_TRACE("mod_hdl = " << reinterpret_cast<std::size_t>(mod_hdl)
                 << ", data_len = " << data_len << ", uri = " <<
                 (uri_obj ? uri_obj->c_str() : (uri_str ? uri_str : "")));
    // check arguments
    if (!mod_hdl || (!uri_obj && !uri_str) || !data || data_len <= 0)
    {
        std::string s;
        add_error_msg(s, !mod_hdl, "invalid module handle");
        add_error_msg(s, !uri_obj && !uri_str, "no URI given");
        add_error_msg(s, !data, "data == NULL");
        add_error_msg(s, data_len <= 0, "data_len <= 0");
        HC_LOG_DEBUG(s);
    }
    else
    {
        tech_module* self = reinterpret_cast<tech_module*>(mod_hdl);
        //if (self->m_recv_buf.try_acquire(data_len))
        //{
            if (uri_obj) self->handle_receive(data, data_len, *uri_obj);
            else
            {
                hamcast::uri u(uri_str);
                self->handle_receive(data, data_len, u);
            }
        //}
        //else
        //{
        //    std::cerr << "PACKAGE DROPPED: tech_module::recv_callback"
        //    << std::endl;
        //}
    }
}

void tech_module::atomic_msg_size_changed(hc_module_instance_handle_t mod_hdl,
                                          size_t new_atomic_size)
{
    HC_LOG_TRACE("");
    if (!mod_hdl)
    {
        HC_LOG_DEBUG("mod_hdl == nullptr");
    }
    else
    {
        tech_module* tm = reinterpret_cast<tech_module*>(mod_hdl);
        boost::lock_guard<boost::shared_mutex> guard(tm->m_mtx);
        tm->m_atomic_msg_size = static_cast<boost::uint32_t>(new_atomic_size);
    }
}

boost::uint32_t tech_module::atomic_msg_size()
{
    boost::shared_lock<boost::shared_mutex> guard(m_mtx);
    return m_atomic_msg_size;
}

hc_module_instance_handle_t
tech_module::new_instance(hc_module_instance_t mod_ptr,
                          hc_module_handle* mod_handle,
                          hc_kvp_list_t* list,
                          size_t init_atomic_size)
{
    HC_LOG_TRACE("");
    tech_module::ptr tm(new tech_module(mod_ptr, mod_handle, list));
    tm->m_atomic_msg_size = init_atomic_size;
    add_instance(tm);
    tm->m_worker = boost::thread(&tech_module::run_worker, tm);
    return tm.get();
}

void tech_module::event_callack(hc_module_instance_handle_t handle,
                                int etype,
                                const hc_uri_t* uri_obj,
                                const char* uri_cstr)
{
    HC_REQUIRE(etype >= 1 && etype <= 3);
    HC_REQUIRE((uri_obj) || (uri_cstr));
    tech_module* self = reinterpret_cast<tech_module*>(handle);
    uri group;
    if (uri_obj) group = *uri_obj;
    else group = uri_cstr;

    membership_event event(group, self->iface_id(),
                           static_cast<membership_event_type>(etype));
    deliver_event(event);
}

void tech_module::handle_kvp_list(hc_kvp_list_t* list)
{
    if (list)
    {
        hc_kvp_list_t& l = *list;
        if (l.key && l.value)
        {
            std::string k(l.key);
            std::string v(l.value);
            m_service_discovery_results[k] = v;
        }
        handle_kvp_list(l.next);
    }
}

tech_module::tech_module(hc_module_instance_t mod_ptr,
                         hc_module_handle* mod_hdl,
                         hc_kvp_list_t* sd_res)
    : ref_count(0), m_handle(mod_hdl), m_instance(mod_ptr), m_id(next_iid())
    , m_atomic_msg_size(0)
{
    handle_kvp_list(sd_res);
}

tech_module::ptr tech_module::instance_by_iid(hamcast::interface_id iid)
{
    read_guard guard(s_instances_mtx);
    instances_by_id::const_iterator i = s_by_id.find(iid);
    if (i != s_by_id.end())
    {
        return i->second;
    }
    else return 0;
}

hamcast::uri tech_module::mapping_of(const hamcast::uri& group_uri)
{
    HC_LOG_TRACE("group = " << group_uri.str());
    util::future<uri> result;
    m_queue.push(new tm_msg(group_uri, &result));
    return result.get();
}

void tech_module::load(const char* module_filename,
                       const std::map<std::string, std::string>& ini_args,
                       size_t max_msg_size)
{
    HC_LOG_TRACE("module_filename = " << module_filename);

    std::vector<hc_kvp_list_t> m_key_value_pairs;
    m_key_value_pairs.resize(ini_args.size());

    // store key value pairs in vector
    {
        std::size_t j = 0;
        for (std::map<std::string, std::string>::const_iterator i = ini_args.begin();
             i != ini_args.end();
             ++i)
        {
            m_key_value_pairs[j].key = i->first.c_str();
            m_key_value_pairs[j].value = i->second.c_str();
            if ((j + 1) < m_key_value_pairs.size())
            {
                m_key_value_pairs[j].next = &(m_key_value_pairs[j + 1]);
            }
            else
            {
                m_key_value_pairs[j].next = 0;
            }
            ++j;
        }
    }

    // open the module file
    void* handle = dlopen(module_filename, RTLD_NOW);

    if (!handle)
    {
        std::string errmsg = "Could not load \"";
        errmsg += module_filename;
        errmsg += "\" (dlopen returned NULL, dlerror() = \"";
        errmsg += dlerror();
        errmsg += "\")";
        HC_LOG_FATAL(errmsg);
        throw could_not_load_module(errmsg);
    }
    check4error(handle, dlerror(), false);

    // load functions
    hc_module_handle* result = (hc_module_handle*) malloc(sizeof(hc_module_handle));

    result->ref_count = 1;
    result->library_handle = handle;

    // make sure ref_count is written
    HC_MEMORY_BARRIER();

    result->map_fun = (hc_map_fun_t) dlsym(handle, "hc_map");
    check4error(handle, dlerror());

    result->sendto_fun = (hc_sendto_fun_t) dlsym(handle, "hc_sendto");
    check4error(handle, dlerror());

    result->leave_fun = (hc_leave_fun_t) dlsym(handle, "hc_leave");
    check4error(handle, dlerror());

    result->join_fun = (hc_join_fun_t) dlsym(handle, "hc_join");
    check4error(handle, dlerror());

    result->group_set_fun = (hc_group_set_fun_t) dlsym(handle, "hc_group_set");
    check4error(handle, dlerror());

    result->neighbor_set_fun = (hc_neighbor_set_fun_t)
                             dlsym(handle, "hc_neighbor_set");
    check4error(handle, dlerror());

    result->children_set_fun = (hc_children_set_fun_t)
                             dlsym(handle, "hc_children_set");
    check4error(handle, dlerror());

    result->parent_set_fun = (hc_parent_set_fun_t)
                           dlsym(handle, "hc_parent_set");
    check4error(handle, dlerror());

    result->designated_host_fun = (hc_designated_host_fun_t)
                                dlsym(handle, "hc_designated_host");
    check4error(handle, dlerror());

    result->shutdown_fun = (hc_shutdown_fun_t) dlsym(handle, "hc_shutdown");
    check4error(handle, dlerror());

    result->delete_instance_fun = (hc_delete_instance_fun_t)
                                  dlsym(handle, "hc_delete_instance");

    hc_init_fun_t init_fun = (hc_init_fun_t) dlsym(handle, "hc_init");
    check4error(handle, dlerror());

    hc_kvp_list_t* list = 0;
    if (!m_key_value_pairs.empty())
    {
        list = &(m_key_value_pairs[0]);
    }

    // init module
    init_fun(hc_get_log_fun(),
             result,
             &tech_module::new_instance,
             &tech_module::recv_callback,
             &tech_module::event_callack,
             &tech_module::atomic_msg_size_changed,
             max_msg_size,
             list);

    // make sure init_fun is done
    HC_MEMORY_BARRIER();
    mh_release mhr;
    mhr(result);

    // done
}

void tech_module::unload_all()
{
    HC_LOG_TRACE("");
    typedef std::vector<tech_module::ptr> module_ptr_vec;
    module_ptr_vec instances;
    // fetch (and clear) all instances
    {
        write_guard guard(s_instances_mtx);
        HC_LOG_DEBUG("s_by_id.count(): " << s_by_id.size());
        for (instances_by_id::iterator i(s_by_id.begin());
             i != s_by_id.end();
             ++i)
        {
            instances.push_back(i->second);
        }
        s_by_id.clear();
    }
    HC_LOG_DEBUG("instances.size(): " << instances.size());
    // no need to unload manually, tech modules are unloaded
    // when reference count drops to zero
}

void tech_module::unload()
{
    HC_LOG_TRACE("id = " << iface_id());
    util::future<bool> fbool;
    m_queue.push(new tm_msg(&fbool, kill_t));
    if (fbool.get())
    {
        m_worker.join();
    }
}

bool tech_module::join(const hamcast::uri& what,
                       const abstract_socket::ptr& who)
{
    HC_LOG_TRACE("id = " << iface_id() << ", group = " << what.str());
    util::future<bool> result;
    m_queue.push(new tm_msg(what, who, &result, join_t));
    return result.get();
}

bool tech_module::leave(const hamcast::uri& what,
                        const abstract_socket::ptr& who)
{
    HC_LOG_TRACE("id = " << iface_id() << ", group = " << what.str());
    if (who)
    {
        util::future<bool> result;
        m_queue.push(new tm_msg(what, who, &result, leave_t));
        return result.get();
    }
    return false;
}

bool tech_module::leave_all(const abstract_socket::ptr& who)
{
    HC_LOG_TRACE("");
    if (who)
    {
        util::future<bool> result;
        m_queue.push(new tm_msg(who, &result));
        return result.get();
    }
    return false;
}

template<size_t Size>
session_msg* new_mcast_packet_msg(const void* data, int data_size, const hamcast::uri& source)
{
    mcast_packet_msg<Size>* tmsg = new mcast_packet_msg<Size>;
    tmsg->data_size = static_cast<size_t>(data_size);
    tmsg->source = source;
    (void) memcpy(tmsg->data, data, data_size);
    return tmsg;
}

void tech_module::handle_receive(const void* data, int len,
                                 const hamcast::uri& source)
{
    HC_LOG_TRACE("source = " << source.str());
    HC_REQUIRE(len >= 0 && data);
    if (len > 65536)
    {
        std::cout << "packet dropped because its size is > 65536" << std::endl;
        return;
    }
    size_t pos = 0;
    size_t num_socks = 0;
    abstract_socket::ptr socks_arr[32];     // up to 32 sockets on stack
    abstract_socket::ptr* socks_vec = NULL; // use heap otherwise
    abstract_socket::ptr* socks = NULL;     // points to either vec or arr
    // get list of proxies
    // lifetime scope of guard
    {
        read_guard guard(m_joins_mtx);
        joins_map::iterator i = m_joins.find(source);
        if (i != m_joins.end())
        {
            socket_ptr_list& lst = i->second.second;
            num_socks = lst.size();
            if (num_socks <= 32)
            {
                socks = socks_arr;
            }
            else
            {
                socks_vec = new abstract_socket::ptr[num_socks];
                socks = socks_vec;
            }
            for (socket_ptr_list::iterator j = lst.begin();
                 j != lst.end();
                 ++j)
            {
                socks[pos++] = *j;
            }
        }
    }
    // "deliver" data
    if (num_socks > 0)
    {
        for (size_t i = 0; i < num_socks; ++i)
        {
            if (socks[i]->acquire_bytes(static_cast<size_t>(len)))
            {
                session_msg* msg;
                if (len < 128) msg = new_mcast_packet_msg<128>(data, len, source);
                else if (len < 256) msg = new_mcast_packet_msg<256>(data, len, source);
                else if (len < 512) msg = new_mcast_packet_msg<512>(data, len, source);
                else if (len < 1024) msg = new_mcast_packet_msg<1024>(data, len, source);
                else if (len < 2048) msg = new_mcast_packet_msg<2048>(data, len, source);
                else if (len < 4096) msg = new_mcast_packet_msg<4096>(data, len, source);
                else if (len < 8192) msg = new_mcast_packet_msg<8192>(data, len, source);
                else if (len < 16384) msg = new_mcast_packet_msg<16384>(data, len, source);
                else if (len < 32768) msg = new_mcast_packet_msg<32768>(data, len, source);
                else msg = new_mcast_packet_msg<65536>(data, len, source);
                msg->sck_id = socks[i]->id();
                socks[i]->parent_session()->push(msg);
            }
            else
            {
                std::cerr << "*** dropped package ***" << std::endl;
            }
        }
    }
    // cleanup
    delete[] socks_vec;

    // get the original URI of @p source
    // get a copy of data and store it in an multicast packet
    /*
    char* data_copy = new char[len];
    (void) memcpy(data_copy, data, len);
    // lifetime scope of guard
    {
        read_guard guard(m_joins_mtx);
        joins_map::iterator i = m_joins.find(source);
        if (i != m_joins.end())
        {
            multicast_packet mp(i->second.first, len, data_copy);
            socket_ptr_list& lst = i->second.second;
            for (socket_ptr_list::iterator j = lst.begin();
                 j != lst.end();
                 ++j)
            {
                (*j)->async_receive(mp);
            }
        }
    }
    */
}

void tech_module::neighbor_set(std::vector<hamcast::uri>& result)
{
    HC_LOG_TRACE("");
    boost::lock_guard<boost::shared_mutex> guard(m_mtx);
    HC_REQUIRE(!closed());
    hc_uri_list_t list = m_handle->neighbor_set_fun(m_instance);
    consume_uri_list(result, list);
    release_uri_list(list);
}

void tech_module::group_set(std::vector<std::pair<hamcast::uri,
                                                  boost::uint32_t> >& result)
{
    HC_LOG_TRACE("");
    boost::lock_guard<boost::shared_mutex> guard(m_mtx);
    HC_REQUIRE(!closed());
    hc_uri_list_t list = m_handle->group_set_fun(m_instance);
    consume_uri_list(result, list);
    release_uri_list(list);
}

void tech_module::children_set(std::vector<hamcast::uri>& result,
                               const hamcast::uri& u)
{
    HC_LOG_TRACE("uri = " << u.str());
    boost::lock_guard<boost::shared_mutex> guard(m_mtx);
    HC_REQUIRE(!closed());
    hc_uri_list_t list = m_handle->children_set_fun(m_instance, &u, u.c_str());
    consume_uri_list(result, list);
    release_uri_list(list);
}

void tech_module::parent_set(std::vector<hamcast::uri>& result,
                             const hamcast::uri& u)
{
    HC_LOG_TRACE("uri = " << u.str());
    boost::lock_guard<boost::shared_mutex> guard(m_mtx);
    HC_REQUIRE(!closed());
    hc_uri_list_t list = m_handle->parent_set_fun(m_instance, &u, u.c_str());
    consume_uri_list(result, list);
    release_uri_list(list);
}

bool tech_module::designated_host(const hamcast::uri& u)
{
    HC_LOG_TRACE("");
    boost::lock_guard<boost::shared_mutex> guard(m_mtx);
    HC_REQUIRE(!closed());
    return m_handle->designated_host_fun(m_instance, &u, u.c_str()) == 1;
}

void tech_module::add_job(const send_job::ptr& job)
{
    m_queue.push(new tm_msg(job));
}

bool tech_module::add(const abstract_socket::ptr& whom,
                      const uri& mapped_what,
                      const uri& unmapped_what)
{
    HC_LOG_TRACE("");
    write_guard guard(m_joins_mtx);
    std::pair<uri, socket_ptr_list>& element = m_joins[mapped_what];
    socket_ptr_list& lst = element.second;
    if (lst.empty())
    {
        element.first = mapped_what;
        m_fjoins[unmapped_what] = mapped_what;
        lst.push_back(whom);
        return true;
    }
    else
    {
        socket_ptr_list::iterator i = std::find(lst.begin(), lst.end(), whom);
        if (i == lst.end())
        {
            lst.push_back(whom);
            return true;
        }
    }
    return false;
}

std::pair<bool, size_t> tech_module::del(const abstract_socket::ptr& whom,
                                         const uri& mapped_what,
                                         const uri& unmapped_what)
{
    HC_LOG_TRACE("");
    write_guard guard(m_joins_mtx);
    joins_map::iterator ji = m_joins.find(mapped_what);
    if (ji == m_joins.end())
    {
        return std::make_pair(false, 0);
    }
    else
    {
        std::pair<uri, socket_ptr_list>& element = ji->second;
        socket_ptr_list& lst = element.second;
        socket_ptr_list::iterator i = std::find(lst.begin(), lst.end(), whom);
        if (i != lst.end())
        {
            lst.erase(i);
            if (lst.empty())
            {
                m_joins.erase(mapped_what);
                m_fjoins.erase(unmapped_what);
            }
            return std::make_pair(true, lst.size());
        }
        return std::make_pair(false, lst.size());
    }
}

void tech_module::del_all(const abstract_socket::ptr& whom)
{
    HC_LOG_TRACE("");
    // caches all groups that are empty after removing whom
    std::set<uri> empty_groups;
    // delete whom from all maps
    // lifetime scope of guard
    {
        write_guard guard(m_joins_mtx);
        for (joins_map::iterator i = m_joins.begin(); i != m_joins.end(); ++i)
        {
            socket_ptr_list& spl = i->second.second;
            socket_ptr_list::iterator j = std::find(spl.begin(),spl.end(),whom);
            if (j != spl.end())
            {
                spl.erase(j);
                if (spl.empty())
                {
                    empty_groups.insert(i->first);
                    m_fjoins.erase(i->second.first);
                }
            }
        }
        // erase from m_joins if needed
        for (std::set<uri>::const_iterator i = empty_groups.begin();
             i != empty_groups.end();
             ++i)
        {
            m_joins.erase(*i);
            HC_REQUIRE(m_handle->leave_fun(m_instance, &(*i), i->c_str())
                       == HC_SUCCESS);
        }
    }
}

void tech_module::worker()
{
    HC_LOG_TRACE("");
    detail::tm_mapping_helper helper(*this);
    joins_map joins;
    util::uri_mappings<> mappings;
    tm_msg* msg = 0;
    for (;;)
    {
        msg = m_queue.pop();
        switch (msg->type)
        {

         case mapping_lookup_t:
            {
                HC_LOG_SCOPE("case mapping_lookup_t",
                             "req_uri = " << msg->req_uri.str());
                uri result = mappings.forward_lookup(msg->req_uri, helper);
                msg->f_uri->set(result);
            }
            break;

         case send_job_t:
            {
                HC_LOG_SCOPE("case send_job_t", "");
                send_job& j = *(msg->jptr);
                try
                {
                    uri receiver = mappings.forward_lookup(j.receiver(),helper);
                    HC_LOG_DEBUG("sendto " << j.receiver().str()
                                 << " (mapped to " << receiver.str() << ")");
                    m_handle->sendto_fun(m_instance,
                                         j.send_message().content(),
                                         j.send_message().content_size(),
                                         j.ttl(),
                                         &receiver,
                                         receiver.c_str());
                }
                catch (hamcast::requirement_failed& e)
                {
                    HC_LOG_ERROR("Error while sending an package "
                                 << "to the URI \""
                                 << j.receiver().str()
                                 << "\"; Requirement failed: "
                                 << e.what());
                }
            }
            break;

         case join_t:
            {
                HC_LOG_SCOPE("case join_t", "");
                uri mapped = mappings.forward_lookup(msg->req_uri, helper);
                bool success = add(msg->sptr, mapped, msg->req_uri);
                if (success)
                {
                    HC_REQUIRE(m_handle->join_fun(m_instance,
                                                  &mapped, mapped.c_str())
                               == HC_SUCCESS);
                    msg->f_bool->set(true);
                }
                else
                {
                    msg->f_bool->set(false);
                }
            }
            break;

         case leave_t:
            {
                HC_LOG_SCOPE("case leave_t", "");
                uri mapped = mappings.forward_lookup(msg->req_uri, helper);
                std::pair<bool, size_t> res = del(msg->sptr,
                                                  mapped, msg->req_uri);
                if (res.first)
                {
                    if (res.second == 0)
                    {
                        HC_REQUIRE(m_handle->leave_fun(m_instance,
                                                       &mapped, mapped.c_str())
                                   == HC_SUCCESS);
                    }
                    msg->f_bool->set(true);
                }
                else
                {
                    msg->f_bool->set(false);
                }
            }
            break;

         case leave_all_t:
            {
                HC_LOG_SCOPE("case leave_all_t", "");
                del_all(msg->sptr);
                msg->f_bool->set(true);
            }
            break;

         case kill_t:
            {
                m_handle->delete_instance_fun(m_instance);
                msg->f_bool->set(true);
                delete msg;
                // exit thread
                return;
            }

         default:
            {
                HC_LOG_SCOPE("default case",
                             "Unknown message type: " << msg->type);
            }
            break;

        }
        delete msg;
    }
}

void tech_module::run_worker(tech_module::ptr self_ptr)
{
    HC_LOG_TRACE("");
    if (!self_ptr) return;
    tech_module& self = *self_ptr;
    self.worker();
}

} } // namespace hamcast::middleware
