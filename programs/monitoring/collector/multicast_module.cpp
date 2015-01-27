#include <iostream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "hamcast/hamcast.hpp"
#include "http_message.hpp"
#include "multicast_module.hpp"

multicast_module::multicast_module(int &update_rate,std::string& group, std::string& addr)
    : m_update_rate(update_rate), m_group(group), m_running(true), m_addr(addr),m_send(false),m_receive(false)
{
}

void multicast_send_thread(multicast_module* multicast_module){
    bool p_running = multicast_module->get_running();
    hamcast::multicast_socket sock;
    std::string msg = multicast_module->create_connect();
    while(p_running){
        sock.send(multicast_module->get_group_uri(),msg.size(),msg.c_str());
        sleep(multicast_module->get_update_rate());
    }
}

std::string multicast_module:: create_connect(){
    std::stringstream output;
    boost::property_tree::ptree pt;
    pt.add("name","connect");
    pt.add("address",m_addr);
    try{
        boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
        boost::property_tree::xml_parser::write_xml( output, pt,w );
    }
    catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return output.str();
}

void multicast_receive_thread(multicast_module* multicast_module){
    bool p_running = multicast_module->get_running();
    hamcast::multicast_socket sock;
    sock.join(multicast_module->get_group_uri());
    while(p_running){
        hamcast::multicast_packet pack;
        sock.try_receive(pack,100);
    }
}

void multicast_module::start_send(){
    try{
     m_thread_send = boost::thread(multicast_send_thread,this);
     m_send= true;
     m_thread_send.join();
    }
    catch(std::exception & e)
    {
        std::cerr << e.what()<< std::endl;
    }
}


void multicast_module::start_receive(){
    try{
        m_thread_receive = boost::thread(multicast_receive_thread,this);
        m_receive = true;
        m_thread_receive.join();
    }
    catch(std::exception& e){
        std::cerr << e.what()<< std::endl;
    }
}

void multicast_module::stop(){

    m_running = false;
    if(m_receive){
      m_thread_receive.join();
    }
    if(m_send){
      m_thread_send.join();
    }

}
