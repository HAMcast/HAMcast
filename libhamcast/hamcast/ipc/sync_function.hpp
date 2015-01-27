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

#ifndef HAMCAST_IPC_SYNC_FUNCTION_HPP
#define HAMCAST_IPC_SYNC_FUNCTION_HPP

#include <boost/function.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_fundamental.hpp>

#include "hamcast/exception.hpp"

#include "hamcast/util/unit.hpp"
#include "hamcast/util/if_else_t.hpp"
#include "hamcast/util/read_buffer.hpp"
#include "hamcast/util/write_buffer.hpp"
#include "hamcast/util/const_buffer.hpp"
#include "hamcast/util/serialization.hpp"

#include "hamcast/ipc/function_id.hpp"
#include "hamcast/ipc/exception_id.hpp"
#include "hamcast/ipc/client_channel.hpp"

namespace hamcast { namespace ipc {

// implementation of an ipc function
template<function_id m_id,
         typename ResultType,
         typename T1 = util::unit, typename T2 = util::unit,
         typename T3 = util::unit, typename T4 = util::unit>
class sync_function_base
{

 public:

    typedef ResultType result_t;

    static const int used_args = util::first_unit<T1,T2,T3,T4>::value;

    typedef typename
            util::if_else_t<boost::is_fundamental<T1>, T1, const T1&>::type
            arg1_t;

    typedef typename
            util::if_else_t<boost::is_fundamental<T2>, T2, const T2&>::type
            arg2_t;

    typedef typename
            util::if_else_t<boost::is_fundamental<T3>, T3, const T3&>::type
            arg3_t;

    typedef typename
            util::if_else_t<boost::is_fundamental<T4>, T4, const T4&>::type
            arg4_t;

    typedef boost::function1<void,result_t&> fun0_t;

    typedef boost::function2<void,result_t&,arg1_t> fun1_t;

    typedef boost::function3<void,result_t&,arg1_t,arg2_t> fun2_t;

    typedef boost::function4<void,result_t&,arg1_t,arg2_t,arg3_t> fun3_t;

    typedef boost::function5<void,result_t&,arg1_t,arg2_t,arg3_t,arg4_t> fun4_t;

 protected:

    // prohibit void functions with no arguments
//    BOOST_STATIC_ASSERT((used_args > 0 || util::is_not_unit<result_t>::value));

    result_t invoke(arg1_t v1 = T1(), arg2_t v2 = T2(),
                    arg3_t v3 = T3(), arg4_t v4 = T4())
    {
        intrusive_ptr<client_channel> cc = client_channel::get();
        message::ptr resp;
        // lifetime scope of wb
        {
            // serialize args as content
            intrusive_ptr<util::write_buffer<> > wb(new util::write_buffer<>);
            // lifetime scope of s
            {
                util::serializer s(wb.get());
                s << v1 << v2 << v3 << v4;
            }
            util::const_buffer cb(wb->size(), wb->data());
            resp = cc->send_sync_request(m_id, cb);
        }
        // this requirement is already enforced by the ctor of srm
        //HC_REQUIRE(resp && resp->type() == sync_response);
        // lifetime scope of d and srm
        {
            sync_response_view srm(*resp);
            util::const_buffer cb(srm.content_size(), srm.content());
            util::deserializer d(new util::read_buffer(cb));
            switch (srm.exc_id())
            {

             case eid_none:
                // scope for local variables
                {
                    result_t result;
                    d >> result;
                    return result;
                }

             case eid_internal_interface_error:
                // scope for local variables
                {
                    std::string err_msg;
                    d >> err_msg;
                    throw internal_interface_error(err_msg);
                }

             case eid_requirement_failed:
                // scope for local variables
                {
                    std::string err_msg;
                    d >> err_msg;
                    throw requirement_failed(err_msg);
                }

             default:
                // scope for local variables
                {
                    HC_LOG_FATAL("Unknown exception ID: " << srm.exc_id());
                    throw std::logic_error("Unknown exception ID");
                }

            }
        }
    }

    /*
     * @brief Reply to the synchronous request @p msg with the result of
     *              the callback @p cb via channel @p ch.
     */
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               void* cb)
    {
        // check arguments
        HC_REQUIRE(cb);
        // get arguments
        T1 arg1;
        T2 arg2;
        T3 arg3;
        T4 arg4;
        // lifetime scope of d
        {
            util::const_buffer cb(msg_content_size, msg_content);
            util::deserializer d(new util::read_buffer(cb));
            d >> arg1 >> arg2 >> arg3 >> arg4;
        }
        // serializer to write content
        intrusive_ptr<util::write_buffer<> > wb(new util::write_buffer<>);
        exception_id eid = eid_none;
        // lifetime scope of "s" and "result"
        {
            util::serializer s(wb.get());
            result_t result;
            // execute callback and store result
            try
            {
                switch (used_args)
                {

                 case 0:
                    (*reinterpret_cast<fun0_t*>(cb))(result);
                    break;

                 case 1:
                    (*reinterpret_cast<fun1_t*>(cb))(result,arg1);
                    break;

                 case 2:
                    (*reinterpret_cast<fun2_t*>(cb))(result,arg1,arg2);
                    break;

                 case 3:
                    (*reinterpret_cast<fun3_t*>(cb))(result,arg1,arg2,arg3);
                    break;

                 case 4:
                    (*reinterpret_cast<fun4_t*>(cb))(result,arg1,arg2,arg3,arg4);
                    break;

                 default: throw std::logic_error("used_args invalid");

                }
                // no exception
                s << result;
            }
            catch (requirement_failed& e)
            {
                HC_LOG_ERROR("requirement_failed: " << e.what());
                eid = eid_requirement_failed;
                s << e.what_str();
            }
            catch (internal_interface_error& e)
            {
                HC_LOG_ERROR("internal_interface_error: " << e.what());
                eid = eid_internal_interface_error;
                s << e.what_str();
            }
        }
        // TODO: überflüssiges message object
        ipc_out << message::create(sync_response, eid, req_id, 0, wb->take());
    }

};

/*
 * Implements a synchronous request via IPC.
 */
template<function_id m_id, typename R,
         typename T1 = util::unit, typename T2 = util::unit,
         typename T3 = util::unit, typename T4 = util::unit>
struct sync_function : sync_function_base<m_id, R, T1, T2, T3, T4>
{
    typedef sync_function_base<m_id, R, T1, T2, T3, T4> super;

    /*
     * @brief Invoke this functor with given arguments.
     * @throw requirement_failed
     * @throw internal_interface_error
     * @returns The result of the IPC function call.
     */
    R operator()(typename super::arg1_t arg1, typename super::arg2_t arg2,
                 typename super::arg3_t arg3, typename super::arg4_t arg4)
    {
        return this->invoke(arg1, arg2, arg3, arg4);
    }

    /*
     * @brief The type of the callback needed by {@link reply()}.
     */
    typedef typename super::fun4_t callback;

    /*
     * @brief Reply to the IPC function call @p msg. The result of @p cb is
     *        send to the client via the channel @p ch.
     */
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               callback& cb)
    {
        super::reply(req_id, msg_content, msg_content_size, ipc_out,
                     reinterpret_cast<void*>(&cb));
    }
};

template<function_id m_id, typename R,
         typename T1, typename T2, typename T3>
struct sync_function<m_id, R, T1, T2, T3, util::unit>
     : sync_function_base<m_id, R, T1, T2, T3>
{
    typedef sync_function_base<m_id, R, T1, T2, T3> super;
    R operator()(typename super::arg1_t arg1, typename super::arg2_t arg2,
                 typename super::arg3_t arg3)
    {
        return this->invoke(arg1, arg2, arg3);
    }
    typedef typename super::fun3_t callback;
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               callback& cb)
    {
        super::reply(req_id, msg_content, msg_content_size, ipc_out,
                     reinterpret_cast<void*>(&cb));
    }
};

template<function_id m_id, typename R, typename T1, typename T2>
struct sync_function<m_id, R, T1, T2, util::unit, util::unit>
     : sync_function_base<m_id, R, T1, T2>
{
    typedef sync_function_base<m_id, R, T1, T2> super;
    R operator()(typename super::arg1_t arg1, typename super::arg2_t arg2)
    {
        return this->invoke(arg1, arg2);
    }
    typedef typename super::fun2_t callback;
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               callback& cb)
    {
        super::reply(req_id, msg_content, msg_content_size, ipc_out,
                     reinterpret_cast<void*>(&cb));
    }
};

template<function_id m_id, typename R, typename T1>
struct sync_function<m_id, R, T1, util::unit, util::unit, util::unit>
     : sync_function_base<m_id, R, T1>
{
    typedef sync_function_base<m_id, R, T1> super;
    R operator()(typename super::arg1_t arg1)
    {
        return this->invoke(arg1);
    }
    typedef typename super::fun1_t callback;
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               callback& cb)
    {
        super::reply(req_id, msg_content, msg_content_size, ipc_out,
                     reinterpret_cast<void*>(&cb));
    }
};

template<function_id m_id, typename R>
struct sync_function<m_id, R, util::unit, util::unit, util::unit, util::unit>
     : sync_function_base<m_id, R>
{
    typedef sync_function_base<m_id, R> super;
    R operator()()
    {
        return this->invoke();
    }
    typedef typename super::fun0_t callback;
    void reply(boost::uint32_t req_id,
               const void* msg_content,
               boost::uint32_t msg_content_size,
               util::serializer& ipc_out,
               callback& cb)
    {
        super::reply(req_id, msg_content, msg_content_size, ipc_out,
                     reinterpret_cast<void*>(&cb));
    }
};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_SYNC_FUNCTION_HPP
