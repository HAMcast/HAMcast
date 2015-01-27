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

#include <memory>
#include <cstddef>
#include <sstream>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

#include "hamcast/util/single_reader_queue.hpp"

using std::cout;
using std::endl;

namespace {

enum msg_type
{
    kill_t,
    send_t
};

struct loopback_module_msg
{

    loopback_module_msg* next;

    msg_type type;

    hamcast::uri source;

    struct
    {
        size_t size;
        char* buf;
    }
    data;

    loopback_module_msg() : next(0), type(kill_t)
    {
        data.size = 0;
        data.buf = 0;
    }

    loopback_module_msg(size_t msize, const void* mbuf, const hamcast::uri& src)
        : next(0), type(send_t), source(src)
    {
        data.size = msize;
        data.buf = new char[msize];
        memcpy(data.buf, mbuf, msize);
    }

    ~loopback_module_msg()
    {
        if (data.buf) delete[] data.buf;
    }

};

struct loopback_module
{

    hamcast::util::single_reader_queue<loopback_module_msg> m_queue;
    hc_module_instance_handle_t m_handle;
    hc_event_callback_t m_event_cb;
    boost::thread m_recv_loop;

    static void recv_loop(loopback_module* _this,
                          hc_module_instance_handle_t hdl,
                          hc_recv_callback_t cb)
    {
        loopback_module_msg* lmm;
        for (;;)
        {
            lmm = _this->m_queue.pop();
            if (lmm->type == kill_t)
            {
                delete lmm;
                return;
            }
            else if (lmm->type == send_t)
            {
                cb(hdl, lmm->data.buf, lmm->data.size,
                   &(lmm->source), lmm->source.c_str());
            }
            delete lmm;
        }
    }

    loopback_module(hc_event_callback_t ev_cb) : m_handle(0), m_event_cb(ev_cb)
    {
    }

    void set_handle(hc_module_instance_handle_t hdl)
    {
        m_handle = hdl;
    }

    void run(hc_module_instance_handle_t hdl, hc_recv_callback_t cb)
    {
        m_recv_loop = boost::thread(&loopback_module::recv_loop, this, hdl, cb);
    }

    ~loopback_module()
    {
        m_queue.push(new loopback_module_msg);
        m_recv_loop.join();
    }

    int join(const hc_uri_t* group_uri, const char* group_uri_str)
    {
        HC_LOG_TRACE("uri = " << group_uri_str);
        if (m_handle)
        {
            m_event_cb(m_handle,
                       static_cast<int>(hamcast::join_event),
                       group_uri,
                       group_uri_str);
        }
        return HC_SUCCESS;
    }

    int leave(const hc_uri_t* group_uri, const char* group_uri_str)
    {
        HC_LOG_TRACE("uri = " << group_uri_str);
        if (m_handle)
        {
            m_event_cb(m_handle,
                       static_cast<int>(hamcast::leave_event),
                       group_uri,
                       group_uri_str);
        }
        return HC_SUCCESS;
    }

    int sendto(const void* buf, int len, unsigned char,
                  const hc_uri_t* src, const char* group_uri_str)
    {
        HC_LOG_TRACE("uri = " << group_uri_str << " (" << len << " bytes)");
        if (src && buf) m_queue.push(new loopback_module_msg(len, buf, *src));
        return HC_SUCCESS;
    }

    hc_uri_result_t map(const hc_uri_t* group_uri, const char* group_uri_str)
    {
        HC_LOG_TRACE("uri = " << group_uri_str);
        hc_uri_result_t result;
        result.uri_obj = new hamcast::uri(*group_uri);
        result.uri_str = 0;
        return result;
    }

    hc_uri_list_t empty_uri_list()
    {
        hc_uri_list_t result;
        result.next = 0;
        result.type = HC_IGNORED;
        result.uri_obj = 0;
        result.uri_str = 0;
        return result;
    }

    hc_uri_list_t neighbor_set()
    {
        return empty_uri_list();
    }

    hc_uri_list_t group_set()
    {
        return empty_uri_list();
    }

    hc_uri_list_t children_set(const hc_uri_t*, const char*)
    {
        return empty_uri_list();
    }

    hc_uri_list_t parent_set(const hc_uri_t*, const char*)
    {
        return empty_uri_list();
    }

    int designated_host(const hc_uri_t*, const char*)
    {
        return HC_IS_NOT_A_DESIGNATED_HOST;
    }

};

inline loopback_module* self(hc_module_instance_t instance)
{
    return reinterpret_cast<loopback_module*>(instance);
}

} // namespace <anonymous>

extern "C" void hc_init(hc_log_fun_t,
                        struct hc_module_handle* mod_handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_atomic_msg_size_callback_t,
                        size_t max_message_size,
                        hc_kvp_list_t*)
{
    loopback_module* _this = new loopback_module(event_cb);

/*
    cout << "void_module initialized with following arguments:" << endl;
    hc_kvp_list_t* kvp = kvp_list;
    while (kvp)
    {
        cout << kvp->key << " = " << kvp->value << endl;
        kvp = kvp->next;
    }
*/
    // build kvp list
    std::string name_key = "if_name";
    std::stringstream name_stream;
    name_stream << "loopback_module(0x"
                << std::hex
                << reinterpret_cast<std::size_t>(_this)
                << ")";
    std::string name_val = name_stream.str();
    std::string addr_key = "if_addr";
    std::string addr_val = "localhost";
    std::string tech_key = "if_tech";
    std::string tech_val = "loopback";

    hc_kvp_list_t name;
    name.key = name_key.c_str();
    name.value = name_val.c_str();
    name.next = 0;

    hc_kvp_list_t addr;
    addr.key = addr_key.c_str();
    addr.value = addr_val.c_str();
    addr.next = &name;

    hc_kvp_list_t tech;
    tech.key = tech_key.c_str();
    tech.value = tech_val.c_str();
    tech.next = &addr;

    hc_module_instance_handle_t hdl = new_instance_cb(_this, mod_handle, &tech, max_message_size);
    _this->set_handle(hdl);
    _this->run(hdl, recv_cb);

}

extern "C" int hc_join(hc_module_instance_t instance,
                       const hc_uri_t* group_uri,
                       const char* uri_str)
{
    return self(instance)->join(group_uri, uri_str);
}

extern "C" int hc_leave(hc_module_instance_t instance,
                        const hc_uri_t* group_uri,
                        const char* uri_str)
{
    return self(instance)->leave(group_uri, uri_str);
}

extern "C" int hc_sendto(hc_module_instance_t instance,
                         const void* buf,
                         int slen,
                         unsigned char ttl,
                         const hc_uri_t* group_uri,
                         const char* uri_str)
{
    return self(instance)->sendto(buf, slen, ttl, group_uri, uri_str);
}

extern "C" void hc_delete_instance(hc_module_instance_t instance)
{
    delete self(instance);
}

extern "C" void hc_shutdown()
{
}

extern "C" hc_uri_result_t hc_map(hc_module_instance_t instance,
                                  const hc_uri_t* group_uri,
                                  const char* uri_str)
{
    return self(instance)->map(group_uri, uri_str);
}

extern "C" hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance)
{
    return self(instance)->neighbor_set();
}

extern "C" hc_uri_list_t hc_group_set(hc_module_instance_t instance)
{
    return self(instance)->group_set();
}

extern "C" hc_uri_list_t hc_children_set(hc_module_instance_t instance,
                                         const hc_uri_t* arg0,
                                         const char* arg1)
{
    return self(instance)->children_set(arg0, arg1);
}

extern "C" hc_uri_list_t hc_parent_set(hc_module_instance_t instance,
                                       const hc_uri_t* arg0,
                                       const char* arg1)
{
    return self(instance)->parent_set(arg0, arg1);
}

extern "C" int hc_designated_host(hc_module_instance_t instance,
                                  const hc_uri_t* arg0,
                                  const char* arg1)
{
    return self(instance)->designated_host(arg0, arg1);
}
