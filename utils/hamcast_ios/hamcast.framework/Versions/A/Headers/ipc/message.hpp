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

#ifndef HAMCAST_IPC_MESSAGE_HPP
#define HAMCAST_IPC_MESSAGE_HPP

#include <new>
#include <limits>
#include <utility>
#include <boost/cstdint.hpp>
#include "hamcast/exception.hpp"
#include "hamcast/socket_id.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/hamcast_logging.h"
#include "hamcast/multicast_packet.hpp"
#include "hamcast/membership_event.hpp"

#include "hamcast/util/comparable.hpp"
#include "hamcast/util/const_buffer.hpp"

#include "hamcast/ipc/stream_id.hpp"
#include "hamcast/ipc/request_id.hpp"
#include "hamcast/ipc/function_id.hpp"
#include "hamcast/ipc/exception_id.hpp"
#include "hamcast/ipc/message_type.hpp"
#include "hamcast/ipc/sequence_number.hpp"

// forward declaration of message
namespace hamcast { namespace ipc { class message; } }

// forward declarations of utility classes
namespace hamcast { namespace util {
struct serializer;
struct deserializer;
} }

namespace hamcast { namespace ipc {

// forward declaration of visitors
struct sync_request_view;
struct sync_response_view;
struct async_send_view;
struct async_recv_view;
struct cumulative_ack_view;
struct retransmit_view;

/**
 * @addtogroup IPC
 * @{
 */

/**
 * @brief Describes a single IPC message.
 *
 * See @ref IPC_struct for detailed description and usage.
 */
class message : public ref_counted, util::comparable<message, message>
{

    friend struct sync_request_view;
    friend struct sync_response_view;
    friend struct async_send_view;
    friend struct async_recv_view;
    friend struct cumulative_ack_view;
    friend struct retransmit_view;

    boost::uint16_t m_type;
    boost::uint16_t m_field1;
    boost::uint32_t m_field2;
    boost::uint32_t m_field3;
    boost::uint32_t m_content_size;
    char* m_content;

    // @warning message takes ownership of @p ct
    inline message(boost::uint16_t mtype,
                   boost::uint16_t mfield1,
                   boost::uint32_t mfield2,
                   boost::uint32_t mfield3,
                   boost::uint32_t ct_size,
                   char* ct)
        : m_type(mtype)
        , m_field1(mfield1)
        , m_field2(mfield2)
        , m_field3(mfield3)
        , m_content_size(ct_size)
        , m_content(ct)
    {
    }

 public:

    int compare(const message& other) const;

    /**
     * @brief The size of an IPC message header in bytes.
     */
    static const size_t header_size =
            (2 * sizeof(boost::uint16_t)) + (3 * sizeof(boost::uint32_t));

    /**
     * @brief A smart pointer to an instance of
     *        {@link hamcast::ipc::message message}.
     */
    typedef intrusive_ptr<message> ptr;

    /**
     * @brief Create an IPC message.
     * @param mtype The type of this IPC message.
     * @param field1 First message field (16 bit).
     * @param field2 Second message field (32 bit).
     * @param field3 Third message field (32 bit).
     * @param ct_size The size of @p ct.
     * @param ct The content of this IPC message.
     * @pre <code>(ct != NULL && ct_size > 0)</code> or
     *      <code>(ct == NULL && ct_size == 0)</code>.
     * @pre @p mtype is a valid message type.
     * @warning ipc_message takes ownership of @p ct if <code>ct != NULL</code>
     * @warning @p ct should be allocated with <code>new char[...]</code>
     *          because the dtor calls <code>delete[] ct;</code>.
     * @returns A smart pointer to the newly created message object.
     */
    static ptr create(message_type mtype,
                      boost::uint16_t field1,
                      boost::uint32_t field2,
                      boost::uint32_t field3,
                      boost::uint32_t ct_size,
                      char* ct);

    /**
     * @brief Create an IPC message.
     *
     * This is a convenient function that calls
     * {@link create()
     *        create(mtype, field1, field2, field3, ct.first, ct.second)}.
     * @param mtype The type of this IPC message.
     * @param field1 First message field (16 bit).
     * @param field2 Second message field (32 bit).
     * @param field3 Third message field (32 bit).
     * @param ct A pair describing the content as {<i>size</i>, <i>data</i>}.
     * @returns A smart pointer to the newly created message object.
     */
    inline static ptr create(message_type mtype,
                             boost::uint16_t field1,
                             boost::uint32_t field2,
                             boost::uint32_t field3,
                             const std::pair<size_t, void*>& ct)
    {
        HC_REQUIRE(ct.first < std::numeric_limits<boost::uint32_t>::max());
        return create(mtype, field1, field2, field3,
                      static_cast<boost::uint32_t>(ct.first),
                      reinterpret_cast<char*>(ct.second));
    }

    static util::serializer& serialize(util::serializer&, const ptr&);

    static util::serializer& serialize(util::serializer&, const message&);

    static util::deserializer& deserialize(util::deserializer&, ptr&);

    static void serialize_as_message(util::serializer& s,
                                     message_type mtype,
                                     boost::uint16_t field1,
                                     boost::uint32_t field2,
                                     boost::uint32_t field3,
                                     boost::uint32_t ct_size,
                                     const void* ct);

    static void serialize_as_message(util::serializer& s,
                                     socket_id receiver_sid,
                                     const multicast_packet& async_recv_packet);

    static void serialize_as_message(util::serializer& s,
                                     const membership_event& event);

    virtual ~message();

    /**
     * @brief Get the value of the <code>message type</code> field.
     * @returns The {@link hamcast::ipc::message_type message_type}
     *         of <code>this</code>.
     */
    inline message_type type() const
    {
        return static_cast<message_type>(m_type);
    }

    /**
     * @brief Get the size of {@link content()}.
     * @returns The size of {@link content()} in bytes.
     */
    inline boost::uint32_t content_size() const { return m_content_size; }

    /**
     * @brief Get the content of this message.
     * @returns The internal C-buffer.
     */
    inline const char* content() const { return m_content; }

    /**
     * @brief Get the full size of this message (content + header).
     * @returns <code>header_size + content_size()</code>
     */
    inline size_t size() const
    {
        return header_size + static_cast<size_t>(content_size());
    }

};

/**
 * @brief Base class of all message views.
 */
class message_view
{

 protected:

    message& msg;
    //::ptr msg;

    message_view(message& msg_ref, message_type) : msg(msg_ref)
    {
//      HC_REQUIRE(msg_ref && msg_ref->type() == mtype);
    }

 public:

    inline message_type type() const
    {
        return msg.type();
    }

    /**
     * @brief Get the size of {@link content()}.
     * @returns The size of {@link content()} in bytes.
     */
    inline boost::uint32_t content_size() const { return msg.content_size(); }

    /**
     * @brief Get the content of this message.
     * @returns A pointer to the internal C-buffer.
     */
    inline const char* content() const { return msg.content(); }

    /**
     * @brief Get the full size of this message (content + header).
     * @returns {@link content_size()} +
     *         {@link hamcast::ipc::message::header_size}.
     */
    inline size_t size() const
    {
        return message::header_size + static_cast<size_t>(msg.content_size());
    }

    /**
     * @brief Get a (smart) pointer to the viewed message.
     * @returns The assigned {@link hamcast::ipc::message::ptr}.
     */
    inline message::ptr message_ptr() const { return &msg; }

};

/**
 * @brief A view for {@link hamcast::ipc::sync_request sync_request} messages.
 */
struct sync_request_view : message_view
{

    inline sync_request_view(message& mref)
        : message_view(mref, sync_request)
    {
    }

    /**
     * @brief Get the ID of the function the client wants to invoke.
     * @returns <code>field1</code> interpreted as
     *         {@link hamcast::ipc::function_id}.
     */
    inline function_id fun_id() const
    {
        return static_cast<function_id>(msg.m_field1);
    }

    /**
     * @brief Get the request ID.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::ipc::request_id}.
     */
    inline request_id req_id() const
    {
        return static_cast<request_id>(msg.m_field2);
    }

    // field3 unused

};

/**
 * @brief A view for {@link hamcast::ipc::sync_response sync_response} messages.
 */
struct sync_response_view : message_view
{

    inline sync_response_view(message& mref)
        : message_view(mref, sync_response)
    {
    }

    /**
     * @brief Get the ID of the occured exception during invocation.
     * @returns <code>field1</code> interpreted as
     *         {@link hamcast::ipc::exception_id}.
     */
    inline exception_id exc_id() const
    {
        return static_cast<exception_id>(msg.m_field1);
    }

    /**
     * @brief Get the request ID.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::ipc::request_id}.
     */
    inline request_id req_id() const
    {
        return static_cast<request_id>(msg.m_field2);
    }

    // field3 unused

};

/**
 * @brief A view for {@link hamcast::ipc::async_send async_send} messages.
 */
struct async_send_view : message_view
{

    inline async_send_view(message& mref)
        : message_view(mref, async_send)
    {
    }

    /**
     * @brief Get the ID of the output stream.
     * @returns <code>field1</code> interpreted as
     *         {@link hamcast::ipc::stream_id}.
     */
    inline stream_id stm_id() const
    {
        return static_cast<stream_id>(msg.m_field1);
    }

    /**
     * @brief Get the ID of the output socket.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::socket_id}.
     */
    inline socket_id sck_id() const
    {
        return static_cast<socket_id>(msg.m_field2);
    }

    /**
     * @brief Get the sequence number of this message packet.
     * @returns <code>field3</code> interpreted as
     *         {@link hamcast::ipc::sequence_number}.
     */
    inline sequence_number seq_nr() const
    {
        return static_cast<sequence_number>(msg.m_field3);
    }

};

/**
 * @brief A view for {@link hamcast::ipc::async_recv async_recv} messages.
 */
struct async_recv_view : message_view
{

    inline async_recv_view(message& mref)
        : message_view(mref, async_recv)
    {
    }

    // field1 unused

    /**
     * @brief Get the ID of the output socket.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::socket_id}.
     */
    inline socket_id sck_id() const
    {
        return static_cast<socket_id>(msg.m_field2);
    }

    // field3 unused

};

/**
 * @brief A view for {@link hamcast::ipc::cumulative_ack cumulative_ack}
 *        messages.
 */
struct cumulative_ack_view : message_view
{

    inline cumulative_ack_view(message& mref)
        : message_view(mref, cumulative_ack)
    {
    }

    /**
     * @brief Get the ID of the output stream.
     * @returns <code>field1</code> interpreted as
     *         {@link hamcast::ipc::stream_id}.
     */
    inline stream_id stm_id() const
    {
        return static_cast<stream_id>(msg.m_field1);
    }

    /**
     * @brief Get the ID of the output socket.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::socket_id}.
     */
    inline socket_id sck_id() const
    {
        return static_cast<socket_id>(msg.m_field2);
    }

    /**
     * @brief Get the ID (sequence_number) of the acknowledged packet.
     * @returns <code>field3</code> interpreted as
     *         {@link hamcast::ipc::sequence_number}.
     */
    inline sequence_number pck_id() const
    {
        return static_cast<sequence_number>(msg.m_field3);
    }

};

/**
 * @brief A view for {@link hamcast::ipc::retransmit retransmit} messages.
 */
struct retransmit_view : message_view
{

    inline retransmit_view(message& mref)
        : message_view(mref, retransmit)
    {
    }

    /**
     * @brief Get the ID of the output stream.
     * @returns <code>field1</code> interpreted as
     *         {@link hamcast::ipc::stream_id}.
     */
    inline stream_id stm_id() const
    {
        return static_cast<stream_id>(msg.m_field1);
    }

    /**
     * @brief Get the ID of the output socket.
     * @returns <code>field2</code> interpreted as
     *         {@link hamcast::socket_id}.
     */
    inline socket_id sck_id() const
    {
        return static_cast<socket_id>(msg.m_field2);
    }

    /**
     * @brief Get the ID (sequence_number) of the acknowledged packet.
     * @returns <code>field3</code> interpreted as
     *         {@link hamcast::ipc::sequence_number}.
     */
    inline sequence_number pck_id() const
    {
        return static_cast<sequence_number>(msg.m_field3);
    }

};

/**
 * @}
 */

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_MESSAGE_HPP
