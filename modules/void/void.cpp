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

#include <cstddef>
#include <sstream>
#include <iostream>

#include <boost/thread.hpp>
#include <boost/preprocessor/stringize.hpp>

#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"

#include "void.hpp"

using std::cout;
using std::endl;

namespace {

hc_uri_list_t empty_uri_list()
{
    hc_uri_list_t result;
    result.next = 0;
    result.type = HC_IGNORED;
    result.uri_obj = 0;
    result.uri_str = 0;
    return result;
}

inline void_module* self(hc_module_instance_t instance)
{
    return reinterpret_cast<void_module*>(instance);
}

} // namespace <anonymous>


void_module::void_module(hc_event_callback_t ecb, hc_recv_callback_t rcb)
    : m_recv(rcb), m_event_cb(ecb), m_handle(0)
{
    generator_running = false;
}

void_module::~void_module()
{
    HC_LOG_TRACE("");
}

void void_module::set_handle(hc_module_instance_handle_t hdl)
{
    m_handle = hdl;
}

void void_module::join(const uri& what)
{
    HC_LOG_TRACE("uri = " << what.str());
    if (m_handle)
    {
        m_event_cb(m_handle,
                   static_cast<int>(hamcast::join_event),
                   &what,
                   what.c_str());
    }
    if (what == "jo:ker")
    {
        boost::lock_guard<boost::mutex> guard(generator_mtx);
        if (generator_running == false)
        {
            // go crazy
            generator_thread = boost::thread(boost::bind(void_module_generator_loop, this));
            generator_running = true;
        }
    }
}

void void_module::leave(const uri& what)
{
    HC_LOG_TRACE("uri = " << what.str());
    if (m_handle)
    {
        m_event_cb(m_handle,
                   static_cast<int>(hamcast::leave_event),
                   &what,
                   what.c_str());
    }
}

void void_module::send(const uri& whom, const void*, int len, unsigned char)
{
    HC_LOG_TRACE("uri = " << whom << " (" << len << " bytes)");
}

hamcast::uri void_module::map(const uri& what)
{
    return what;
}

void void_module::generator_loop()
{
    hamcast::uri joker("jo:ker");
    std::string quote("Ever danced with the devil in the pale moonlight?");
    for (;;)
    {
        // generate messages at the speed of light
        m_recv(m_handle, quote.c_str(), quote.size(), &joker, joker.c_str());
    }
}

void void_module::neighbor_set(std::vector<uri>&) { }

void void_module::group_set(std::vector<std::pair<uri, int> >&) { }

void void_module::children_set(std::vector<uri>&, const uri&) { }

void void_module::parent_set(std::vector<uri>&, const uri&) { }

bool void_module::designated_host(const uri&) { return false; }

void void_module_generator_loop(void_module* self)
{
    self->generator_loop();
}

extern "C" void hc_init(hc_log_fun_t log_fun,
                        struct hc_module_handle* handle,
                        hc_new_instance_callback_t new_instance_cb,
                        hc_recv_callback_t recv_cb,
                        hc_event_callback_t event_cb,
                        hc_atomic_msg_size_callback_t,
                        size_t max_message_size,
                        hc_kvp_list_t*)
{
    hc_set_log_fun(log_fun);

    void_module* _this = new void_module(event_cb, recv_cb);

    // build kvp list
    std::string name_key = "if_name";
    std::stringstream name_stream;
    name_stream << "void_module(0x"
                << std::hex
                << reinterpret_cast<std::size_t>(_this)
                << ")";
    std::string name_val = name_stream.str();
    std::string addr_key = "if_addr";
    std::string addr_val = "localhost";
    std::string tech_key = "if_tech";
    std::string tech_val = "none";

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

    hc_module_instance_handle_t hdl = new_instance_cb(_this, handle, &tech, max_message_size);
    _this->set_handle(hdl);

}

extern "C" int hc_join(hc_module_instance_t instance,
                       const hc_uri_t* group_uri,
                       const char*)
{
    self(instance)->join(*group_uri);
    return HC_SUCCESS;
}

extern "C" int hc_leave(hc_module_instance_t instance,
                        const hc_uri_t* group_uri,
                        const char*)
{
    self(instance)->leave(*group_uri);
    return HC_SUCCESS;
}

extern "C" int hc_sendto(hc_module_instance_t instance,
                         const void* buf,
                         int slen,
                         unsigned char ttl,
                         const hc_uri_t* group_uri,
                         const char*)
{
    self(instance)->send(*group_uri, buf, slen, ttl);
    return HC_SUCCESS;
}

extern "C" void hc_delete_instance(hc_module_instance_t instance)
{
    delete self(instance);
}

extern "C" void hc_shutdown()
{
}

extern "C" hc_uri_result_t hc_map(hc_module_instance_t,
                                  const hc_uri_t* group_uri,
                                  const char*)
{
    hc_uri_result_t result;
    result.uri_obj = new hamcast::uri(*group_uri);
    result.uri_str = NULL;
    return result;
}

extern "C" hc_uri_list_t hc_neighbor_set(hc_module_instance_t)
{
    return empty_uri_list();
}

extern "C" hc_uri_list_t hc_group_set(hc_module_instance_t)
{
    return empty_uri_list();
}

extern "C" hc_uri_list_t hc_children_set(hc_module_instance_t,
                                         const hc_uri_t*,
                                         const char*)
{
    return empty_uri_list();
}

extern "C" hc_uri_list_t hc_parent_set(hc_module_instance_t,
                                       const hc_uri_t*,
                                       const char*)
{
    return empty_uri_list();
}

extern "C" int hc_designated_host(hc_module_instance_t,
                                  const hc_uri_t*,
                                  const char*)
{
    return HC_IS_NOT_A_DESIGNATED_HOST;
}
