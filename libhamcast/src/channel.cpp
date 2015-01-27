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

#include <cstdio>
#include <poll.h>

#include <boost/thread.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/ipc/channel.hpp"
#include "hamcast/multicast_socket.hpp"
#include "hamcast/util/id_generator.hpp"
#include "hamcast/util/serialization.hpp"

#ifndef POLLRDHUP
#define POLLRDHUP POLLHUP
#endif

namespace hamcast { namespace ipc {

void channel::set_err_str(const char* cstr)
{
    // do only set the first occured error
    if (cstr && m_err_str.empty()) m_err_str = cstr;
}

void channel::handle_poll_timeout()
{
}

void channel::loop()
{
    HC_LOG_TRACE("m_source->read_handle() = " << m_source->read_handle()
                 << ", m_sink->write_handle() = " << m_sink->write_handle());
    m_loop_done = false;
    util::native_socket ipc_fd  = m_source->read_handle();
    util::native_socket pipe_fd = m_pipe->read_handle();
    std::vector<pollfd> fds;
    { // lifetime scope of temporary variables
        pollfd tmp_fd;
        tmp_fd.fd = pipe_fd;
        tmp_fd.events = POLLIN;
        tmp_fd.revents = 0;
        fds.push_back(tmp_fd);
        if (ipc_fd != util::invalid_socket)
        {
            tmp_fd.fd = ipc_fd;
            tmp_fd.events = POLLIN;
            tmp_fd.revents = 0;
            fds.push_back(tmp_fd);
        }
        else
        {
            HC_LOG_INFO("skipped IPC socket (invalid)");
        }
    }
    while (m_loop_done == false)
    {
        int presult = poll(fds.data(), fds.size(), m_poll_timeout);
        if (presult < 0)
        {
            switch (errno)
            {
             case EINTR:
                // a signal was caught, just try again
                break;
             default:
             {
                HC_LOG_FATAL("poll() failed");
                perror("poll() failed");
                abort();
             }
            }
        }
        else if (presult == 0)
        {
            handle_poll_timeout();
        }
        else
        {
            for (std::vector<pollfd>::iterator i = fds.begin(); i != fds.end(); ++i)
            {
                if (i->revents & (POLLRDHUP | POLLERR | POLLHUP | POLLNVAL))
                {
                    if (i->fd == ipc_fd)
                    {
                        HC_LOG_FATAL("IPC connection lost");
                    }
                    else
                    {
                        HC_LOG_FATAL("pipe error");
                    }
                    return;
                }
                else if (i->revents & (POLLIN | POLLPRI))
                {
                    if (i->fd == ipc_fd)
                    {
                        ipc_read();
                    }
                    else
                    {
                        char tmp[64];
                        size_t num = m_pipe->read_some(64, tmp);
                        poll_messages(num);
                    }
                }
                i->revents = 0;
            }
        }
    }
}

void channel::run_loop(channel* self_ptr)
{
    HC_LOG_TRACE("self_ptr = " << self_ptr);
    HC_REQUIRE(self_ptr != 0);
    channel::ptr scoped_ptr(self_ptr);
    HC_MEMORY_BARRIER();
    channel& self = *self_ptr;
    try
    {
        self.loop();
    }
    catch (std::exception& e)
    {
        std::cout << "exception in channel: " << typeid(e).name() << ": " << e.what() << std::endl;
        self.set_err_str(e.what());
    }
    catch (...)
    {
        self.set_err_str("Unknown Exception in send_loop.");
    }
    self.close();
    self.on_exit(self.m_err_str);
}

void channel::notify_message(size_t num)
{
    HC_REQUIRE(num < 32);
    char dummy[32];
    m_pipe->write(num, dummy);
}

void channel::close()
{
    m_source->close();
    m_sink->close();
}

void channel::on_exit(const std::string&)
{
}

channel::channel(const util::source::ptr& in, const util::sink::ptr& out, int tout)
    : m_sink(out), m_source(in), m_pipe(util::socket_io::create_pipe())
    , m_poll_timeout(tout)
{
}

channel::channel(const std::pair<util::source::ptr, util::sink::ptr>& in_out)
    : m_sink(in_out.second), m_source(in_out.first)
    , m_pipe(util::socket_io::create_pipe()), m_poll_timeout(-1)
{
}

void channel::run()
{
    m_loop = boost::thread(boost::bind(&channel::run_loop, this));
}

channel::~channel()
{
    // this implicitly kills the loop
    close();
    //m_loop.join();
}

} } // namespace hamcast::ipc
