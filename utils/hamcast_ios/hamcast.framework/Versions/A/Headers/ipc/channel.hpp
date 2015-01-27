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

#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/ipc/message.hpp"
#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/multicast_packet.hpp"

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

    // binary data sink
    util::sink::ptr m_sink;

    // binary data source
    util::source::ptr m_source;

    /**
     * @brief Creates a channel that reads from @p in and writes to @p out.
     * @param in The input source for @c this.
     * @param out The output sink for @c this.
     */
    channel(const util::source::ptr& in, const util::sink::ptr& out);

    /**
     * @brief Creates a channel that reads from @c io_pair.first
     *        and writes to <code>io_pair.second</code>.
     * @param io_pair @c io_pair.first is the input source for
     *                @c this and @c io_pair.second
     *                is the output sink for @c this.
     */
    channel(const std::pair<util::source::ptr, util::sink::ptr>& io_pair);

    /**
     * @brief Sends IPC data (executed in an own thread).
     */
    virtual void send_loop() = 0;

    /**
     * @brief Receives IPC data (executed in an own thread).
     */
    virtual void receive_loop() = 0;

    /**
     * @brief Override this function if you want to run any cleanup
     *        code after the receive and send loop have exited.
     * @param err_str Occurred error.
     */
    virtual void on_exit(const std::string& err_str);

    // run send and receive loop
    void run();

 public:

    virtual ~channel();

 private:

    // handshake between send and receive loop on startup
    // and before on_exit() is called
    boost::barrier m_barrier;
    // thread object of the send loop
    boost::thread m_send_loop;
    // thread object of the receive loop
    boost::thread m_receive_loop;
    // error string to pass to on_exit()
    std::string m_err_str;
    // guards m_err_str
    boost::mutex m_err_str_mtx;
    // setter
    void set_err_str(const char* cstr);
    // closes m_sink and m_source
    void close();

    // receives IPC messages from m_io
    static void run_send_loop(channel* self);
    // sends IPC messages to m_io
    static void run_receive_loop(channel* self);

};

} } // namespace hamcast::ipc

#endif // HAMCAST_IPC_CHANNEL_HPP
