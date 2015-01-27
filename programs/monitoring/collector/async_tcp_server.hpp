#ifndef ASYNC_TCP_SERVER_HPP
#define ASYNC_TCP_SERVER_HPP

#include <map>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

#include "tcp_session.hpp"
#include "collector.hpp"
#include "function_wrapper.hpp"

class async_tcp_server
{
private:
    boost::asio::io_service&                                m_io_service;
    boost::asio::ip::tcp::acceptor                          m_acceptor;
    boost::thread                                           m_thread;
    int                                                     m_update_rate;
    collector                                               m_collector;
    bool                                                    m_first;
    function_wrapper                                        m_method_caller;

    void handle_accept(tcp_session *old_session,const boost::system::error_code& error);

public:
    /**
      * @brief creates a async tcp server that can handel multiple tcp connections
      * @param port = server port
      * @param update_rate = time value (in seconds) for the tcp_connection update timer
      */
    async_tcp_server(boost::asio::io_service& io_service,int& port,int& update_rate);
    /**
      * @brief Method to start the server thread
      */
    void start();

    void take_node(boost::shared_ptr<node> p_node, std::string daemon_id);
    std::string invoke_call_pointer(std::string name, std::vector<std::string> args);
    void error_handler(std::string id);
    /**
      * @brief terminating the server thread
      */
    void stop();

    inline std::string get_address(){
        return m_acceptor.local_endpoint().address().to_string();
    }

    inline boost::asio::io_service& get_io_service(){
        return m_io_service;
    }
};

#endif // ASYNC_TCP_SERVER_HPP


