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

#ifndef TECH_MODULE_HPP
#define TECH_MODULE_HPP

#include <set>
#include <map>
#include <cmath>
#include <string>
#include <vector>
#include <cstdlib>
#include <stdexcept>

#include "send_job.hpp"
#include "rw_spinlock.hpp"
#include "abstract_socket.hpp"

#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/interface_id.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/util/future.hpp"
#include "hamcast/util/uri_mappings.hpp"
#include "hamcast/util/atomic_operations.hpp"
#include "hamcast/util/single_reader_queue.hpp"

// forward declaration
namespace hamcast { namespace middleware { class tech_module; } }

extern "C" {

// must have external C linkage (forward declared in hamcast_module.h)
struct hc_module_handle
{
    // reference count for hamcast::intrusive_ptr
    volatile boost::int32_t ref_count;
    // pointer to (.so | .dylib | .dll) file
    void* library_handle;
    // function pointers
    hc_map_fun_t map_fun;
    hc_init_fun_t init_fun;
    hc_join_fun_t join_fun;
    hc_leave_fun_t leave_fun;
    hc_sendto_fun_t sendto_fun;
    hc_group_set_fun_t group_set_fun;
    hc_neighbor_set_fun_t neighbor_set_fun;
    hc_children_set_fun_t children_set_fun;
    hc_parent_set_fun_t parent_set_fun;
    hc_designated_host_fun_t designated_host_fun;
    hc_shutdown_fun_t shutdown_fun;
    hc_delete_instance_fun_t delete_instance_fun;
};

} // extern "C"

namespace hamcast { namespace middleware { namespace detail {

struct tm_mapping_helper;

} } } // namespace hamcast::middleware::detail

namespace hamcast { namespace middleware {

struct mh_add_ref
{
    inline void operator()(hc_module_handle* mh)
    {
        if (mh) hamcast::util::add_and_fetch(&(mh->ref_count), 1);
    }
};

struct mh_release
{
    void operator()(hc_module_handle* mh);
};

/**
 * @brief A map with strings as key and values.
 */
typedef std::map<std::string, std::string> string_map;

/**
 * @brief This exception is thrown to indicate that a technology module
 *        could not be loaded from a given shared library file.
 */
struct could_not_load_module : public std::runtime_error
{
    explicit could_not_load_module(const std::string& reason);
};

enum tm_msg_t
{
    mapping_lookup_t,
    send_job_t,
    join_t,
    leave_t,
    leave_all_t,
    kill_t
};

struct tm_msg
{

    tm_msg* next;
    tm_msg_t type;
    uri req_uri;
    union
    {
        util::future<uri>* f_uri;
        util::future<bool>* f_bool;
        struct
        {
            char* data;
            int data_len;
        }
        rcv;
    };
    send_job::ptr jptr;
    abstract_socket::ptr sptr;

    inline tm_msg(util::future<bool>* res, tm_msg_t req_op)
        : next(0), type(req_op), req_uri(), f_bool(res)
    {
        HC_REQUIRE(res && req_op == kill_t);
    }

    inline tm_msg(const uri& what,
                  const abstract_socket::ptr& who,
                  util::future<bool>* res,
                  tm_msg_t req_op)
        : next(0), type(req_op), req_uri(what), f_bool(res), jptr(), sptr(who)
    {
        HC_REQUIRE(req_op == join_t || req_op == leave_t);
    }

    inline tm_msg(const abstract_socket::ptr& who, util::future<bool>* res)
        : next(0), type(leave_all_t), f_bool(res), sptr(who)
    {
    }

    inline tm_msg(const uri& req, util::future<uri>* res)
        : next(0), type(mapping_lookup_t), req_uri(req), f_uri(res)
    {
    }

    inline tm_msg(const send_job::ptr& sjob)
        : next(0), type(send_job_t), req_uri(), f_uri(0), jptr(sjob)
    {
    }

};

/**
 * @brief Encapsulates a multicast technology module.
 */
class tech_module
{

    friend struct detail::tm_mapping_helper;

    static void recv_callback(hc_module_instance_handle_t,
                              const void*, int,
                              const hc_uri_t*, const char*);

    static hc_module_instance_handle_t new_instance(hc_module_instance_t,
                                                    hc_module_handle*,
                                                    hc_kvp_list_t*,
                                                    size_t);

    static void atomic_msg_size_changed(hc_module_instance_handle_t, size_t);

    void handle_receive(const void* data, int len, const hamcast::uri& group);

    void handle_kvp_list(hc_kvp_list_t* list);

    volatile boost::int32_t ref_count;

    intrusive_ptr<hc_module_handle, mh_add_ref, mh_release> m_handle;

    // pointer to the module instance
    hc_module_instance_t m_instance;

    // id of this tech. module instance
    hamcast::interface_id m_id;

    // current atomic message size
    boost::uint32_t m_atomic_msg_size;

    // key values pairs of the service discovery as a std::map
    string_map m_service_discovery_results;

    // technology independend URIs to tech_module specific ones
    util::uri_mappings<boost::shared_mutex> m_mappings;

    util::single_reader_queue<tm_msg> m_queue;

    // private default constructor
    inline tech_module(const char*) { }

    // this thread runs send_worker()
    boost::thread m_worker;

    // send jobs are rejected as long as !m_worker_is_running
    bool m_worker_is_running;

    // cv to wait for new jobs
    boost::condition_variable m_jobs_cv;

    // this mutex guards all other member functions except join(), leave()
    // and handle_receive()
    boost::shared_mutex m_mtx;

    // this mutex protects join(), leave() and handle_receive()
    boost::shared_mutex m_listeners_mtx;

    typedef std::list<abstract_socket::ptr> socket_ptr_list;

    // key = joined group (mapped!)
    // value.first = unmapped key
    // value.second = list of joined sockets
    typedef std::map<uri, std::pair<uri, socket_ptr_list> > joins_map;

    // key = unmapped uri
    // value = mapped uri
    typedef std::map<uri, uri> forward_joins;

    // mapped uri => { original uri, list<abstract_socket> }
    joins_map m_joins;
    // original uri => mapped uri
    forward_joins m_fjoins;
    // guards m_joins and m_fjoins
    rw_spinlock m_joins_mtx;

    inline bool closed() const { return m_handle == 0; }

    tech_module(hc_module_instance_t, hc_module_handle*, hc_kvp_list_t*);

    // result = true if whom was added to m_joins
    bool add(const abstract_socket::ptr& whom,
             const uri& mapped_what,
             const uri& unmapped_what);

    // result.first = true if whom was erased from m_joins; otherwise false
    // result.second = remaining joins for given group
    std::pair<bool, size_t> del(const abstract_socket::ptr& whom,
                                const uri& mapped_what,
                                const uri& unmapped_what);

    // number of erase-operations to delete @p whom from all groups
    // @warning only allowed from worker()
    void del_all(const abstract_socket::ptr& whom);

 public:

    struct tm_add_ref;
    struct tm_release;

    friend struct tm_add_ref;
    friend struct tm_release;

    struct tm_add_ref
    {
        inline void operator()(tech_module* tm)
        {
            if (tm) hamcast::util::add_and_fetch(&(tm->ref_count), 1);
        }
    };

    struct tm_release
    {
        inline void operator()(tech_module* tm)
        {
            if (tm)
            {
                boost::int32_t rc = hamcast::util::add_and_fetch(&(tm->ref_count), -1);
                if (rc == 0)
                {
                    // blocks until tm is unload
                    tm->unload();
                    delete tm;
                }
            }
        }
    };

    typedef intrusive_ptr<tech_module, tm_add_ref, tm_release> ptr;

    ~tech_module();

    inline hc_module_instance_t mod_ptr() const { return m_instance; }

    inline hamcast::interface_id iface_id() const { return m_id; }

    inline const string_map& service_discovery_results() const
    {
        return m_service_discovery_results;
    }

    /**
     * @brief Add a new job to the internal job list.
     */
    void add_job(const send_job::ptr& job);

    /**
     * @brief Get the technology specific possible mappings of @p group_uri.
     */
    hamcast::uri mapping_of(const hamcast::uri& group_uri);

    /**
     * @brief Join the multicast group specified by @p group_uri.
     */
    bool join(const hamcast::uri& what, const abstract_socket::ptr& who);

    /**
     * @brief Leave the multicast group specified by @p group_uri.
     */
    bool leave(const hamcast::uri& group_uri, const abstract_socket::ptr& who);

    boost::uint32_t atomic_msg_size();

    /**
     * @brief Leave all groups for @p who.
     */
    bool leave_all(const abstract_socket::ptr& who);

    void neighbor_set(std::vector<hamcast::uri>& result);

    void group_set(std::vector<std::pair<hamcast::uri, boost::uint32_t> >& res);

    void children_set(std::vector<hamcast::uri>& result, const hamcast::uri& u);

    void parent_set(std::vector<hamcast::uri>& result, const hamcast::uri& u);

    bool designated_host(const hamcast::uri& u);

    /**
     * @brief Unload this instance (stop both worker threads).
     */
    void unload();

    /**
     * @brief Load the module @p module_filename.
     * @warning This function blocks until the initialization and
     *			service discovery of the module are done.
     * @throws could_not_load_module_exception
     */
    static void load(const char* module_filename,
                     const std::map<std::string, std::string>& ini_args,
                     size_t max_msg_size);

    static void unload_all();

    static std::vector<ptr> instances();

    static ptr instance_by_iid(hamcast::interface_id);

    static void event_callack(hc_module_instance_handle_t handle,
                              int etype,
                              const hc_uri_t* uri_obj,
                              const char* uri_cstr);

 private:

    void worker();

    static void run_worker(ptr self_ptr);

    bool send(const hamcast::uri& whom, const void* what, int len, int ttl);

};

} } // namespace hamcast::middleware

#endif // TECH_MODULE_HPP
