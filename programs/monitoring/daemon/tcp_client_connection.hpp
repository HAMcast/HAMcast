#ifndef TCP_CLIENT_CONNECTION_HPP
#define TCP_CLIENT_CONNECTION_HPP
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "http_message.hpp"
#include "method_caller.hpp"

class tcp_client_connection
{
private:

    static const int                max_lenght = 1024;
    boost::asio::io_service&        m_io_service;
    std::string                     m_server_address;
    int                             m_server_port;
    std::string                     m_daemon_id;
    boost::asio::ip::tcp::socket    m_socket;
    boost::asio::ip::tcp::resolver  m_resolver;
    char                            m_request[max_lenght];
    char                            m_reply[max_lenght];

    void handle_connect(const boost::system::error_code& err);
    void handle_write(const boost::system::error_code& err);
    void handle_read(const boost::system::error_code& err,size_t bytes_transferred);

public:

    bool                            m_disconnected;

    /**
      * @brief creates a tcp connection to the given server
      * @param io_service = boost io_service
      * @param server_address = ip address or url of the server
      * @param server_port
      */
    tcp_client_connection(boost::asio::io_service& io_service,std::string& server_address,int& server_port, const std::string& daemon_id);

};

#endif // TCP_CLIENT_CONNECTION_HPP
