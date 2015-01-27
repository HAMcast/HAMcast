#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "http_message.hpp"
#include "function_wrapper.hpp"
#include "tcp_client_connection.hpp"

tcp_client_connection::tcp_client_connection(boost::asio::io_service& io_service,std::string& server_address,
                                             int& server_port, const std::string& daemon_id)
 :m_io_service(io_service), m_server_address(server_address), m_server_port(server_port),m_socket(io_service),
                                                 m_resolver(io_service),m_daemon_id(daemon_id), m_disconnected(false)
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(m_server_address),
                                            m_server_port);
    m_socket.async_connect(endpoint, boost::bind(&tcp_client_connection::handle_connect,
                                                 this,boost::asio::placeholders::error));
}

void tcp_client_connection::handle_connect(const boost::system::error_code& err){
    if (!err)
    {
        std::vector<std::string> payload;
        payload.push_back(m_daemon_id);
        std::string p =m_daemon_id+"\r\n";
        http_message reply("200",p);
        std::string data = reply.to_string();
        boost::asio::async_write( m_socket, boost::asio::buffer(data,data.size()),
                                  boost::bind( &tcp_client_connection::handle_write,this, boost::asio::placeholders::error));
    }
    else
    {
        std::cerr << "Error: " << err.message() << std::endl;
        if (err == boost::asio::error::eof){
            std::cout << "disconnected" << std::endl;
            m_disconnected = true;
        }
    }
}

void tcp_client_connection::handle_write(const boost::system::error_code& err){
    if (!err)
    {
        m_socket.async_read_some(boost::asio::buffer(m_request,max_lenght),
                                 boost::bind(&tcp_client_connection::handle_read,
                                             this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cerr << "Error: " << err.message() << std::endl;
        if (err == boost::asio::error::eof){
            std::cout << "disconnected" << std::endl;
            m_disconnected = true;
        }
    }
}

void tcp_client_connection::handle_read(const boost::system::error_code& err, size_t bytes_transferred){
    if(!err){
        std::cout << "received request from: " << m_server_address << std::endl;
        http_message msg(m_request);
        function_wrapper caller(m_daemon_id);
        std::string meth = msg.get_method();
        std::vector<std::string> args = msg.get_arguments();
        std::string ret = caller.invoke_function(meth,args);
        if(!ret.empty()){
            http_message reply("200",ret);
            std::string data = reply.to_string();
            std::cout << "sending reply to : "+ m_server_address << std::endl;
            boost::asio::async_write( m_socket, boost::asio::buffer(data,data.size()),
                                  boost::bind( &tcp_client_connection::handle_write,this, boost::asio::placeholders::error));
        }
    }
    else{
       std::cerr << "Error: " << err.message() << std::endl;
       if (err == boost::asio::error::eof){
           std::cout << "disconnected" << std::endl;
           m_disconnected = true;
       }
    }

}


