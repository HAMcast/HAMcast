#include <iostream>
#include <string>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "async_tcp_server.hpp"
#include "tcp_session.hpp"

using boost::asio::ip::tcp;

async_tcp_server::async_tcp_server(boost::asio::io_service& io_service,int& port, int& update_rate)
    : m_io_service(io_service),m_acceptor(m_io_service, tcp::endpoint(tcp::v4(), port)), m_update_rate(update_rate),m_first(false), m_method_caller(m_collector)
{
    tcp_session* new_session = new tcp_session(m_io_service, m_update_rate,
                                               boost::bind(&async_tcp_server::take_node,this, _1, _2),
                                               boost::bind(&async_tcp_server::invoke_call_pointer, this, _1, _2),
                                               boost::bind(&async_tcp_server::error_handler,this, _1));
    m_acceptor.async_accept(new_session->socket(),
                            boost::bind(&async_tcp_server::handle_accept, this, new_session,
                                       boost::asio::placeholders::error));
}

int server_thread(async_tcp_server* async_server){
    try
    {
        boost::asio::io_service& io_service = async_server->get_io_service();
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

void async_tcp_server::start(){
    m_thread = boost::thread(server_thread,this);
}

void async_tcp_server::take_node(boost::shared_ptr<node> p_node, std::string daemon_id)
{
    m_collector.deliver_node(p_node,daemon_id);
}

std::string async_tcp_server::invoke_call_pointer(std::string name, std::vector<std::string> args)
{
  return  m_method_caller.invoke_function(name,args);
}

void async_tcp_server::error_handler(std::string id)
{
    m_collector.remove_node(id);
}

void async_tcp_server::stop(){
    m_io_service.stop();
}

void async_tcp_server::handle_accept(tcp_session* old_session,
                                     const boost::system::error_code& error)
{
    if (!error)
    {
         old_session->receive();
         tcp_session* new_session = new tcp_session(m_io_service, m_update_rate,
                                                    boost::bind(&async_tcp_server::take_node,this, _1, _2),
                                                    boost::bind(&async_tcp_server::invoke_call_pointer, this, _1, _2),
                                                    boost::bind(&async_tcp_server::error_handler,this, _1));
        m_acceptor.async_accept(new_session->socket(),
                                boost::bind(&async_tcp_server::handle_accept, this, new_session,
                                            boost::asio::placeholders::error));
    }
    else
    {
        delete old_session;
    }
}
