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

#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>
#include "hamcast/util/socket_io.hpp"
#include "hamcast/util/buffered_sink.hpp"
#include "hamcast/util/buffered_source.hpp"

#ifdef WIN_32
#else
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace hamcast { namespace util {

class native_socket_io : public socket_io
{

    // stores closed state
    volatile bool m_closed;

    native_socket m_wr;
    native_socket m_rd;

    int m_mss;

    static inline int read_mss(int sockfd)
    {
        int result;
        socklen_t mss_len = sizeof(result);
        getsockopt(sockfd, IPPROTO_TCP, TCP_MAXSEG, &result, &mss_len);
        return result;
    }

    static inline bool set_tcp_nodelay(native_socket sockfd)
    {
        int flag = 1;
        // set TCP_NODELAY
        return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
                          &flag, sizeof(int)) == 0;
    }

 public:

    native_socket_io(native_socket ns) : m_closed(false), m_wr(ns), m_rd(ns)
    {
        if (!set_tcp_nodelay(m_wr))
        {
            HC_LOG_WARN("Unable to set TCP_NODELAY");
        }
        m_mss = read_mss(m_wr);
    }

    native_socket_io(native_socket rd, native_socket wr) : m_closed(false), m_wr(wr), m_rd(rd)
    {
        // used for pipes
        m_mss = 0;
    }

    ~native_socket_io()
    {
        close();
    }

    bool wait_for_data(long sec, long usec)
    {
        HC_REQUIRE(!closed());
        struct timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_rd, &rfds);
        int retval = select(m_rd + 1, &rfds, 0, 0, &tv);
        HC_REQUIRE(retval != -1);
        return retval != 0;
    }

    virtual int flush_hint() const
    {
        return m_mss;
    }

    void write(size_t buf_size, const void* buf)
    {
        HC_LOG_TRACE("buf_size: " << buf_size);
        ssize_t bsize = static_cast<ssize_t>(buf_size);
        HC_REQUIRE(bsize >= 0 && buf != NULL);
        if (!m_closed)
        {
            //if (bsize != send(m_wr, buf, buf_size, 0))
            if (bsize != ::write(m_wr, buf, buf_size))
            {
                std::string err = "Cannot write to socket";
                HC_LOG_DEBUG(err);
                throw std::ios_base::failure(err);
            }
        }
    }

    size_t read_some(size_t max, void* storage)
    {
        HC_LOG_TRACE("m_rd = " << m_rd);
        if (max == 0 || !storage) return 0;
        //ssize_t res = recv(m_rd, storage, max, 0);
        ssize_t res = ::read(m_rd, storage, max);
        if (res <= 0)
        {
            std::string err;
            if (res == 0) err = "lost connection!";
            else
            {
                err = "read failure: ";
                switch (errno)
                {
                 case EAGAIN: err += "EAGAIN"; break;
                 case EBADF: err += "EBADF"; break;
                 case ECONNREFUSED: err += "ECONNREFUSED"; break;
                 case EFAULT: err += "EFAULT"; break;
                 case EINTR: err += "EINTR"; break;
                 case EINVAL: err += "EINVAL"; break;
                 case ENOMEM: err += "ENOMEM"; break;
                 case ENOTCONN: err += "ENOTCONN"; break;
                 case ENOTSOCK: err += "ENOTSOCK"; break;
                 default: err += "UNKNOWN_ERROR"; break;
                }
            }
            HC_LOG_DEBUG(err);
            throw std::ios_base::failure(err);
        }
        return static_cast<size_t>(res);
    }

    void read(size_t buf_size, void* buf)
    {
        if (buf_size == 0 || !buf) return;
        size_t written = 0;
        while (written < buf_size)
        {
            written += read_some(buf_size - written,
                                 reinterpret_cast<char*>(buf) + written);
        }
    }

    void close()
    {
        if (!m_closed)
        {
            closesocket(m_wr);
            if (m_wr != m_rd) closesocket(m_wr);
            m_closed = true;
        }
    }

    bool closed() const
    {
        return m_closed;
    }

    void flush () { }

    native_socket read_handle() const
    {
        return m_rd;
    }

    native_socket write_handle() const
    {
        return m_wr;
    }

    bool has_buffered_data() const
    {
        return false;
    }

};

socket_io::ptr socket_io::create(const char* host, boost::uint16_t port)
{
    HC_REQUIRE(host != NULL);
    HC_LOG_TRACE("host = " << host << ", port = " << port);
    native_socket fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    memset(reinterpret_cast<void*>(&serv_addr), 0, sizeof(sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(host);
    serv_addr.sin_port = htons(port);
    // connect to server
    if (0 != connect(fd, reinterpret_cast<sockaddr*>(&serv_addr),
                     sizeof(sockaddr_in)))
    {
        std::string err = "Could not connect to middleware. Is it running?";
        HC_LOG_FATAL(err);
        abort();
    }
    return create(fd);
}

socket_io::ptr socket_io::create(native_socket s)
{
    HC_LOG_TRACE("s = " << s);
    return new native_socket_io(s);
}

socket_io::ptr socket_io::create_pipe()
{
    HC_LOG_TRACE("");
    int fds[2];
    if (pipe(fds) == -1) {
        HC_LOG_FATAL("pipe() failed");
        perror("pipe");
        abort();
    }
    HC_LOG_DEBUG("read = " << fds[0] << ", write = " << fds[1]);
    return new native_socket_io(fds[0], fds[1]);
}

} } // namespace hamcast::ipc
