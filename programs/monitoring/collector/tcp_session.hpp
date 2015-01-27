#ifndef TCP_CONNECTION_HPP
#define TCP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "node.hpp"
#include "interface.hpp"
#include "group.hpp"
#include "http_message.hpp"
#include "collector.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>



class async_tcp_server;
class tcp_session;
const int max_length = 100000;

typedef boost::function<void (boost::shared_ptr<node> node, std::string daemon_id)> method_call;
typedef boost::function<std::string (std::string name, std::vector<std::string> args)> method_invoke;
typedef boost::function<void (std::string)> error_call;

class tcp_session
{

private:

    char*                            m_data;
    std::string                      m_tmp;
    int                              m_received;
    int                              m_deadline;
    std::string                      m_daemon_id;

    method_call                      m_func;
    method_invoke                    m_invoke;
    error_call                       m_error_call;

    void handle_write(const boost::system::error_code& error);

    void handle_timeout(const boost::system::error_code& error);

    void read_message(const boost::system::error_code& error,size_t bytes_transferred);

    void data_received(size_t bytes_transferred);

public:
    boost::asio::ip::tcp::socket     m_socket;
    boost::asio::deadline_timer      m_timer;
    boost::asio::deadline_timer      m_timeout_timer;
    bool                             m_first;
/**
  * @brief create a tcp_connection object
  * @param io_service = io_service that handels async calls
  * @param time = timeout time (in seconds) for the update timer
  */
    tcp_session(boost::asio::io_service& io_service, int& time,method_call func, method_invoke invoke, error_call err_c);

    /**
      * @brief receive data from socket
      */
     void receive();
     void close(const boost::system::error_code& e);

     /**
       * @brief receive data from socket and calls the given message handler
       * @param pointer to tcp_server
       * @param message handler
       */
     void send_data(http_message& data);
     void handle_read();
    inline boost::asio::ip::tcp::socket& socket()
        {
            return m_socket;
        }

    inline void set_daemon_id(std::string& id){
        m_daemon_id = id;
    }

    inline std::string& get_daemon_id(){
        return m_daemon_id;
    }
};

#endif // TCP_CONNECTION_HPP
