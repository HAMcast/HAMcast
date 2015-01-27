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

#ifndef HAMCAST_IPC_CHANNEL_HPP
#define HAMCAST_IPC_CHANNEL_HPP

#include <boost/thread.hpp>

#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/multicast_packet.hpp"

#include "hamcast/ipc/message.hpp"

#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/socket_io.hpp"

namespace hamcast { namespace ipc {

// this class is not part of the "public" part of libhamcast.

/**
 * @brief An IPC channel.
 *
 * This class describes an abstract IPC channel that manages an
 * IPC connection.
 *
 * @note An IPC channel runs two threads (for sending and receiving
 *       IPC messages from its socket).
 */
class channel : public ref_counted
{

 public:

    /**
     * @brief A smart pointer to an instance of
     *        {@link hamcast::ipc::channel channel}.
     */
    typedef intrusive_ptr<channel> ptr;

 protected:

    /**
     * @brief Creates a channel that reads from @p in and writes to @p out.
     * @param in The input source for @c this.
     * @param out The output sink for @c this.
     */
    channel(const util::source::ptr& in, const util::sink::ptr& out, int ms_poll_timeout = -1);

    /**
     * @brief Creates a channel that reads from @c io_pair.first
     *        and writes to <code>io_pair.second</code>.
     * @param io_pair @c io_pair.first is the input source for
     *                @c this and @c io_pair.second
     *                is the output sink for @c this.
     */
    channel(const std::pair<util::source::ptr, util::sink::ptr>& io_pair);

    /**
     * @brief Override this function if you want to run any cleanup
     *        code after the receive and send loop have exited.
     * @param err_str Occurred error.
     */
    virtual void on_exit(const std::string& err_str);

    // run send and receive loop
    void run();

    /**
     * @brief Reads data form the IPC channel via @p m_source.
     */
    virtual void ipc_read() = 0;

    /**
     * @brief Reads @p num messages from the channel's queue.
     */
    virtual void poll_messages(size_t num) = 0;

    virtual void handle_poll_timeout();

    /**
     * @brief Notifies the channel about a new message.
     * @post @p num < 32
     */
    void notify_message(size_t num = 1);

    // binary data sink
    util::sink::ptr m_sink;

    // binary data source
    util::source::ptr m_source;

    bool m_loop_done;

 public:

    virtual ~channel();

 private:

    // thread object of the send loop
    boost::thread m_loop;
    // error string to pass to on_exit()
    std::string m_err_str;
    // setter
    void set_err_str(const char* cstr);
    // closes m_sink and m_source
    void close();

    // receives IPC messages from m_io
    static void run_loop(channel* self);

    void loop();

    // pipe
    util::socket_io::ptr m_pipe;

    int m_poll_timeout;

};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_CHANNEL_HPP
