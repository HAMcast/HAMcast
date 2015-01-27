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

#ifndef HAMCAST_MODULE_H
#define HAMCAST_MODULE_H

#include "hamcast/hamcast_logging.h"

#ifdef __cplusplus
#   include <cstddef>
#	include "hamcast/hamcast.hpp"
    typedef hamcast::uri hc_uri_t;
#else
#   include <stddef.h>
    typedef struct hc_uri_s hc_uri_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def HC_SUCCESS
 * @brief Indicates that a function call returned without error.
 */
#define HC_SUCCESS                0

/**
 * @def HC_UNKNOWN_ERROR
 * @brief Indicates that a function results in an internal or unknown error.
 */
#define HC_UNKNOWN_ERROR          -1

/**
 * @def HC_INVALID_URI
 * @brief Indicates that an URI argument was invalid.
 */
#define HC_INVALID_URI            -2

/**
 * @brief Describes a list of key/value pairs.
 */
typedef struct hc_kvp_list_s
{
    const char* key;
    const char* value;
    struct hc_kvp_list_s* next;
}
hc_kvp_list_t;

/**
 * @brief Describes a handle to a module (dynamic library).
 */
struct hc_module_handle;

/**
 * @brief Describes a handle to a module instance that's associated with
 *        a hc_module_handle and a hc_module_instance_t.
 */
typedef void* hc_module_instance_handle_t;

/**
 * @brief Describes a module instance.
 */
typedef void* hc_module_instance_t;

/**
 * @brief Describes a callback that registers a new module instance.
 */
typedef hc_module_instance_handle_t
        (*hc_new_instance_callback_t)(hc_module_instance_t,
                                      struct hc_module_handle*,
                                      hc_kvp_list_t*,
                                      size_t /* initial atomic msg size */);

/**
 * @brief Describes a callback that handles membership events.
 *
 * The int parameter denotes the type of the event:
 * - 1: join event
 * - 2: leave event
 * - 3: new source event
 *
 * The second or third argument denotes the related group name
 * (either as a {@link hamcast::uri} object or as C-string.
 *
 * The caller is responsible for any memory handling.
 */
typedef void (*hc_event_callback_t)(hc_module_instance_handle_t,
                                    int,
                                    const hc_uri_t*,
                                    const char*);

/**
 * @brief Describes a callback that handles message receives.
 */
typedef void (*hc_recv_callback_t)(hc_module_instance_handle_t,
                                   const void*, int,
                                   const hc_uri_t*, const char*);

typedef void (*hc_atomic_msg_size_callback_t)(hc_module_instance_handle_t,
                                              size_t);

/**
 * @brief Initialize the module.
 *
 * This function is called after the module was successfully
 * loaded and before any other function is called.
 *
 * @param log_fun The logging function this module has to use.
 * @param new_instance_cb The callback to register module instances.
 * @param recv_cb The callback that handles message receives.
 * @param changed_cb The callback that should be invoked if the atomic message
 *                   size of a module instance has changed.
 * @param max_message_size The maximum message size each module has to ensure.
 * @param ini_arguments The INI configuration parameters.
 * @param event_cb The callback that handles membership events.
 */
void hc_init(hc_log_fun_t log_fun,
             struct hc_module_handle*,
             hc_new_instance_callback_t new_instance_cb,
             hc_recv_callback_t recv_cb,
             hc_event_callback_t ,
             hc_atomic_msg_size_callback_t changed_cb,
             size_t max_message_size,
             hc_kvp_list_t* ini_arguments);

/**
 * @brief A function pointer with the signature of hc_init.
 */
typedef void (*hc_init_fun_t)(hc_log_fun_t,
                              struct hc_module_handle*,
                              hc_new_instance_callback_t,
                              hc_recv_callback_t,
                              hc_event_callback_t,
                              hc_atomic_msg_size_callback_t,
                              size_t,
                              hc_kvp_list_t*);

/**
 * @brief Delete given instance.
 */
void hc_delete_instance(hc_module_instance_t);

typedef void (*hc_delete_instance_fun_t)(hc_module_instance_t);

/**
 * @brief Shut module down.
 *
 * The HAMcast middleware asserts, that hc_shutdown is not called unless
 * all registered instances are deleted (hc_delete_instance).
 */
void hc_shutdown();

typedef void (*hc_shutdown_fun_t)();

/**
 * @brief Join the group described by @p group_uri.
 * @param instance_ptr The affected module instance.
 * @param group_uri URI of the multicast
 *                  group (e.g.: "ip://224.2.2.2:5000",
 *                               "ip://224.2.2.2:5000@124.1.14.1",
 *                               "sip://news@cnn.com", ... ).
 * @param group_uri_str The group_uri as C-string.
 * @returns 0 on success; otherwise -1
 */
int hc_join(hc_module_instance_t instance_ptr,
            const hc_uri_t* group_uri,
            const char* );

/**
 * @brief A function pointer with the signature of hc_join.
 */
typedef int (*hc_join_fun_t)(hc_module_instance_t,
                             const hc_uri_t*, const char*);

/**
 * @brief Leaves the group described by @p group_uri.
 * If the group_uri includes an identifier and the module support SSM an
 * SSM-leave will be executed instead of an group-leave.
 * @param instance_ptr The affected module instance.
 * @param group_url URL of the multicast
 *                  group (e.g.: "ipv4://224.2.2.2:5000",
 *                               "ipv4://224.2.2.2:5000@124.1.14.1",
 *                               "sip://news@cnn.com", ... ).
 * @returns 0 on success; otherwise -1
 */
int hc_leave(hc_module_instance_t instance_ptr,
             const hc_uri_t* group_uri,
             const char* );

/**
 * @brief A function pointer with the signature of hc_leave().
 */
typedef int (*hc_leave_fun_t)(hc_module_instance_t,
                              const hc_uri_t*,
                              const char*);

/**
 * @brief Transmit a message the multicast group described by @p group_uri.
 * @param instance_ptr The affected module instance.
 * @param buf Outgoing multicast data.
 * @param len Size of @p buf.
 * @param ttl Maximum hop count for this message.
 * @param group_url URL of the multicast group.
 * @param group_uri_str @p group_url as C-string representation.
 * @returns The number of bytes sent. On error, -1 is returned.
 */
int hc_sendto(hc_module_instance_t instance_ptr,
              const void* buf, int len, unsigned char ttl,
              const hc_uri_t* group_uri, const char* );

/**
 * @brief A function pointer with the signature of hc_sendto().
 */
typedef int (*hc_sendto_fun_t)(hc_module_instance_t,
                               const void*, int, unsigned char,
                               const hc_uri_t*, const char*);

/**
 * @brief Describes an URI either given as C-string or as hc_uri_t.
 *
 * @warning <code>uri_str</code> must be allocated with
 *          <code>malloc()</code>, if set,
 *          because it will be released with <code>free()</code>.
 */
typedef struct
{
    char* uri_str;
    hc_uri_t* uri_obj;
}
hc_uri_result_t;

/**
 * @brief Map a given URI to technology specific URI(s) if possible.
 *
 * The result might only set one of @c uri_str or
 * @c uri_obj. If the module is implemented in C++ then it's
 * strongly recommended to always use only @c uri_obj and set
 * @c uri_str to @c NULL.
 *
 * If set, @c uri_obj will be relased with @c free()
 * and thus must be allocated with malloc().
 *
 * @param instance_ptr The affected module instance.
 *
 * @note This function must return a hc_uri_list_t with
 *       <code>uri_str == NULL</code>, <code>uri_obj == NULL</code>
 *       if the given URI could not be mapped.
 */
hc_uri_result_t hc_map(hc_module_instance_t instance_ptr,
                       const hc_uri_t* group_uri, const char* );

/**
 * @brief A function pointer with the signature of {@link hc_map}.
 */
typedef hc_uri_result_t (*hc_map_fun_t)(hc_module_instance_t,
                                        const hc_uri_t*,
                                        const char*);

#define HC_IGNORED -1
#define HC_LISTENER_STATE 0
#define HC_SENDER_STATE 1
#define HC_SENDER_AND_LISTENER_STATE 2

/**
 * @brief Describes a list of URIs.
 *
 * A list is empty if all fields are set to NULL.
 *
 * @warning <code>uri_str</code> and <code>next</code> must be allocated with
 *          <code>malloc()</code>, if set,
 *          because they'll be released with <code>free()</code>.
 */
typedef struct hc_uri_list_s
{
    /**
     * @brief URI encoded as C-string
     */
    char* uri_str;
    /**
     * @brief URI as hc_uri_t
     */
    hc_uri_t* uri_obj;
    /**
     * @brief One of HC_LISTENER_STATE, HC_SENDER_STATE
     *        or HC_SENDER_AND_LISTENER_STATE.
     *
     * This flag is evaluated by hc_group_set() only.
     * All other calls should set this flag to HC_IGNORED by default.
     */
    int type;
    /**
     * @brief Pointer to the next element in the list
     */
    struct hc_uri_list_s* next;
}
hc_uri_list_t;

/**
 * @brief Get the multicast routing neighbors.
 *
 * All elements in the resulting list should have set <code>type</code> to
 * {@link HC_IGNORED}.
 */
hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance_ptr);

typedef hc_uri_list_t (*hc_neighbor_set_fun_t)(hc_module_instance_t);

/**
 * @brief Get the multicast routing neighbors.
 *
 * All elements in the resulting list should have set <code>type</code> to
 * {@link HC_IGNORED}.
 */
hc_uri_list_t hc_children_set(hc_module_instance_t instance_ptr,
                              const hc_uri_t*, const char*);

typedef hc_uri_list_t (*hc_children_set_fun_t)(hc_module_instance_t,
                                               const hc_uri_t*, const char*);

/**
 * @brief Get the multicast routing neighbors.
 *
 * All elements in the resulting list should have set <code>type</code> to
 * {@link HC_IGNORED}.
 */
hc_uri_list_t hc_parent_set(hc_module_instance_t instance_ptr,
                            const hc_uri_t*, const char*);

typedef hc_uri_list_t (*hc_parent_set_fun_t)(hc_module_instance_t,
                                             const hc_uri_t*, const char*);

/**
 * @brief Get the all (known) registered multicast groups.
 */
hc_uri_list_t hc_group_set(hc_module_instance_t instance_ptr);

typedef hc_uri_list_t (*hc_group_set_fun_t)(hc_module_instance_t);

#define HC_IS_A_DESIGNATED_HOST 1
#define HC_IS_NOT_A_DESIGNATED_HOST 0

/**
 * @brief Inquire wheter the host has the role of a designated forwarder
 *        resp. querier, or not.
 *
 * @returns {@link HC_IS_A_DESIGNATED_HOST} if the host has the role of a
 *         designated forwarder resp. querier;
 *         otherwise {@link HC_IS_NOT_A_DESIGNATED_HOST}
 */
int hc_designated_host(hc_module_instance_t instance_ptr,
                       const hc_uri_t*, const char*);

typedef int (*hc_designated_host_fun_t)(hc_module_instance_t instance_ptr,
                                        const hc_uri_t*, const char*);

#ifdef __cplusplus
}

#include <vector>
#include <utility>

namespace hamcast {

inline void append_from_uri_list(std::vector<uri>& storage, hc_uri_list_t& l)
{
    if (l.uri_obj) storage.push_back(*(l.uri_obj));
    else if (l.uri_str) storage.push_back(hamcast::uri(l.uri_str));
}

template<typename Integer>
inline void append_from_uri_list(std::vector<std::pair<uri, Integer> >& storage,
                                 hc_uri_list_t& l)
{
    if (l.uri_obj) storage.push_back(std::make_pair(*(l.uri_obj), l.type));
    else if (l.uri_str) storage.push_back(std::make_pair(uri(l.uri_str), l.type));
}

template<typename T>
inline void consume_uri_list(std::vector<T>& storage, hc_uri_list_t& l)
{
    append_from_uri_list(storage, l);
    if (l.next) consume_uri_list(storage, *(l.next));
}

inline void set_uri_list_element(hc_uri_list_t* storage, const uri& element)
{
    storage->type = HC_IGNORED;
    storage->uri_obj = new uri(element);
    storage->uri_str = 0;
}

template<typename Integer>
inline void set_uri_list_element(hc_uri_list_t* storage,
                                 const std::pair<uri, Integer>& element)
{
    storage->type = element.second;
    storage->uri_obj = new uri(element.first);
    storage->uri_str = 0;
}

template<typename T>
inline hc_uri_list_t create_uri_list(const std::vector<T>& vec)
{
    hc_uri_list_t result;
    result.next = 0;
    result.type = HC_IGNORED;
    result.uri_obj = 0;
    result.uri_str = 0;
    if (!vec.empty())
    {
        typename std::vector<T>::const_iterator i = vec.begin();
        hc_uri_list_t* tail = &result;
        set_uri_list_element(tail, *i);
        ++i;
        for (; i != vec.end(); ++i)
        {
            hc_uri_list_t* tmp = (hc_uri_list_t*) malloc(sizeof(hc_uri_list_t));
            tmp->next = 0;
            set_uri_list_element(tmp, *i);
            tail->next = tmp;
            tail = tmp;
        }
    }
    return result;
}

inline void release_uri_list(hc_uri_list_t& l)
{
    if (l.next)
    {
        // first release children
        release_uri_list(*(l.next));
        free(l.next);
    }
    if (l.uri_obj) delete l.uri_obj;
    if (l.uri_str) free(l.uri_str);
}

inline uri get_uri_result(hc_uri_result_t& res)
{
    if (res.uri_obj)
    {
        return *(res.uri_obj);
    }
    else if (res.uri_str)
    {
        return uri(res.uri_str);
    }
    else
    {
        return uri();
    }
}

inline hc_uri_result_t create_uri_result(const uri& from)
{
    hc_uri_result_t res;
    res.uri_str = 0;
    res.uri_obj = 0;
    if (!from.empty()) 
        res.uri_obj = new uri(from);
    return res;
}

inline void release_uri_result(hc_uri_result_t& res)
{
    if (res.uri_str) free(res.uri_str);
    if (res.uri_obj) delete res.uri_obj;
}

} // namespace hamcast
#endif

#endif // HAMCAST_MODULE_H
