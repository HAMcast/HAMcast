#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "collector.hpp"
#include "group.hpp"
#include "http_message.hpp"
#include "interface.hpp"
#include "node.hpp"
#include "tcp_session.hpp"

tcp_session::tcp_session(boost::asio::io_service &io_service, int& time, method_call func,method_invoke invoke, error_call err_c)
    : m_socket(io_service), m_timer(io_service,boost::posix_time::seconds(time)),m_deadline(time),m_first(false),
      m_func(func), m_invoke(invoke), m_error_call(err_c), m_timeout_timer(io_service)
{
    m_data = new char[max_length];
    m_tmp = new char[max_length];
}

void tcp_session::receive(){
    m_tmp.clear();
    m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
                               boost::bind(&tcp_session::read_message, this,
                                           boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
     m_timeout_timer.expires_from_now(boost::posix_time::seconds(m_deadline+10));
     m_timeout_timer.async_wait(boost::bind(&tcp_session::close, this, boost::asio::placeholders::error));

}

void tcp_session::close(const boost::system::error_code& e){
 	
	//std::cerr << "connction timeout" << std::endl;
	if(e != boost::asio::error::operation_aborted){
		std::cerr << "connction timeout" << std::endl;        
	//m_timer.cancel();
        //m_timeout_timer.cancel();        
        m_socket.close();
        //m_error_call(m_daemon_id);
	//delete this;
	}
}
void tcp_session::handle_read(){
        m_first = true;
        m_timer.expires_from_now(boost::posix_time::seconds(m_deadline));
        m_timer.async_wait(boost::bind(&tcp_session::handle_timeout,this,boost::asio::placeholders::error));
}

void tcp_session::handle_write(const boost::system::error_code& error){
m_timeout_timer.cancel();      
if(!error){
        receive();
    }
    else{
        std::cerr << "connction failed" << std::endl;
        m_timer.cancel();
        m_socket.close();
        m_error_call(m_daemon_id);
        delete this;
    }
}

void tcp_session:: handle_timeout(const boost::system::error_code& error){
m_timeout_timer.cancel();      
if(m_daemon_id== "monitor"){
        return;
    }
    if(!error){
        std::string meth = "/get_node";
        std::vector<std::string> vec;
        http_message msg(meth,vec);
        std::string data = msg.to_string();
        boost::asio::async_write( m_socket, boost::asio::buffer(data,data.size()),
                                  boost::bind( &tcp_session::handle_write,this, boost::asio::placeholders::error));
	m_timeout_timer.expires_from_now(boost::posix_time::seconds(m_deadline+10));
     m_timeout_timer.async_wait(boost::bind(&tcp_session::close, this, boost::asio::placeholders::error));
    }
    else{
        m_timer.cancel();
        m_socket.close();
        if (error == boost::asio::error::eof){
            std::cout << "disconnected" << std::endl;
        }
        m_error_call(m_daemon_id);
        delete this;
    }
}

void tcp_session::read_message(const boost::system::error_code &error,size_t bytes_transferred)
{
    m_timeout_timer.cancel();  
    std::string received_data(m_data,bytes_transferred);
    m_tmp = m_tmp +received_data;
    http_message h(m_data);
    int a = h.get_header_size();
    int b = h.get_payload_size();
    int c = a+b;
    if(error){
        std::cout << "socket error" << std::endl;
        m_timer.cancel();
        m_socket.close();
        m_error_call(m_daemon_id);
        delete this;
        return;
    }
    if(m_tmp.length() < c ){
        m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
                                 boost::bind(&tcp_session::read_message, this,
                                 boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
	m_timeout_timer.expires_from_now(boost::posix_time::seconds(m_deadline+10));
     m_timeout_timer.async_wait(boost::bind(&tcp_session::close, this, boost::asio::placeholders::error));
    }
    else{
        data_received(bytes_transferred);
    }
}

void tcp_session:: send_data(http_message& data){
    std::string msg = data.to_string();
    boost::asio::async_write( m_socket, boost::asio::buffer(msg,msg.size()),
                              boost::bind( &tcp_session::handle_write,this, boost::asio::placeholders::error));
     m_timeout_timer.expires_from_now(boost::posix_time::seconds(m_deadline+10));
     m_timeout_timer.async_wait(boost::bind(&tcp_session::close, this, boost::asio::placeholders::error));
}

void tcp_session::data_received(size_t bytes_transferred){
        std::string received_data(m_data,bytes_transferred);
        http_message msg(received_data);
        std::cout << "received data from : "+ m_daemon_id << std::endl;
        if(m_first == false){
            if(!msg.get_method().empty()){
                std::string daemon_id = "monitor";
                m_first = true;
                std::string reply_data = m_invoke(msg.get_method(),msg.get_arguments());
                http_message reply("200",reply_data);
                m_daemon_id = daemon_id;
                send_data(reply);
                return;
            }
            else{
                std::string pay =msg.get_payload();
                std::string daemon_id = pay.substr(0,pay.size()-1);
                m_first = true;
                m_daemon_id = daemon_id;
                std::cout << "received data from : "+ m_daemon_id << std::endl;
            }
        }
        else{
            if(!msg.get_method().empty()){
                std::string reply_data = m_invoke(msg.get_method(),msg.get_arguments());
                http_message reply("200",reply_data);
                send_data(reply);
                return;
            }
            if(msg.get_type() == "OK"){
                std::string pay =msg.get_payload();
                boost::shared_ptr<node> n = boost::shared_ptr<node>(new node(pay));
                m_func(n,m_daemon_id);
            }
            if(msg.get_type() == "404"){
                std::cerr << "method not found" << std::endl;
            }
        }
        handle_read();
}
