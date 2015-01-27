#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


#include "hamcast/hamcast.hpp"
#include "http_message.hpp"
#include "method_caller.hpp"
#include "multicast_module.hpp"
#include "tcp_client_connection.hpp"

using std::string;
using std::map;
using std::vector;
using hamcast::uri;
using hamcast::interface_id;

void multicast_send_thread(multicast_module* multicast_module){
    bool p_running = multicast_module->get_running();
    hamcast::multicast_socket sock;
    interface_id ifid = multicast_module->get_interface();
    if (ifid > 0) {
        sock.set_interface(ifid);
    }
    string meth = "/connect";
    vector<string> args;
    args.push_back(multicast_module->get_addr());
    while(p_running){
        http_message http_msg(meth,args);
        string msg = http_msg.to_string();
        try{
            sock.send(multicast_module->get_group_uri(),msg.size(),msg.c_str());
        }
        catch(std::exception& e){
            std::cerr << e.what()<< std::endl;
        }

        sleep(multicast_module->get_update_rate());
    }
}


void multicast_receive_thread(multicast_module* multicast_module){
    std::cout << "start recv thread" << std::endl;
    bool p_running = multicast_module->get_running();
    hamcast::multicast_socket sock;
    interface_id ifid = multicast_module->get_interface();
    if (ifid > 0) {
        sock.set_interface(ifid);
    }
    uri u(multicast_module->get_group_uri());
    std::cout << "Monitoring group: " << u.str() << std::endl;
    hamcast::multicast_packet pack;

    std::cout << u.c_str() << std::endl;
    sock.join(u);
    int counter = 0;
    while(p_running){
        ++counter;
        std::cout << "Loop: " << counter << std::endl;
        try{
            //bool received = sock.try_receive(pack,100);
            //if(received){
            pack = sock.receive();
                const  char* msg = reinterpret_cast< const  char*>(pack.data());
                std::string ms = std::string(msg,0,pack.size());
                std:: string address = multicast_module->parse_connect(ms);
                std::cout << "/connect received" << std::endl;
                size_t f = address.find(":");
                if(f !=std::string::npos){
                    std::string addr = address.substr(0,f);
                    int port = boost::lexical_cast<int>(address.substr(f+1,address.size()-(f+1)));
                    multicast_module->connect_to_server(addr,port);
                }
            //}
        }
        catch(std::exception& e){
            std::cerr << e.what()<< std::endl;
        }

    }
}

std::string multicast_module:: parse_connect(std::string msg){
    std::stringstream input(msg);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return pt.get("address","");
}

void try_io_service(multicast_module* mul){
    boost::asio::io_service& io_service =   mul->get_io_service();
    while(true){
        try{
            io_service.run();
            io_service.reset();
        }
        catch(std::exception& e){
            std::cerr << e.what() << std::endl;
        }
        usleep(100);
    }
}

void multicast_module::connect_to_server(string& addr, int& port){

    map<string, boost::shared_ptr<tcp_client_connection> >::iterator it;
    it= m_server_connections.find(addr);
    if(it == m_server_connections.end()){
        boost::shared_ptr<tcp_client_connection> conn(new tcp_client_connection(m_io_service,addr,port,m_addr));
        m_server_connections.insert(std::pair<std::string,boost::shared_ptr<tcp_client_connection> >(addr,conn));
        if(m_service == false){
             m_io_service_thread = boost::thread(try_io_service,this);
             m_service = true;
        }
    }
    else{
        boost::shared_ptr<tcp_client_connection> c = (*it).second;
        if(c->m_disconnected){
            m_server_connections.erase(it);
            boost::shared_ptr<tcp_client_connection> conn(new tcp_client_connection(m_io_service,addr,port,m_addr));
            m_server_connections.insert(std::pair<string,boost::shared_ptr<tcp_client_connection> >(addr,conn));
            if(m_service == false){
                 m_io_service_thread = boost::thread(try_io_service,this);
                 m_service = true;
            }
        }
        else{
           std::cerr << "allready connected to : " <<  addr << std::endl;
    }
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
