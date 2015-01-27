#ifndef MULTICAST_MODULE_HPP
#define MULTICAST_MODULE_HPP

#include <iostream>
#include <map>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"
#include "http_message.hpp"
#include "method_caller.hpp"
#include "tcp_client_connection.hpp"

class multicast_module
{

private:
    boost::asio::io_service&                m_io_service;
    int                                     m_update_rate;
    boost::thread                           m_thread_send;
    boost::thread                           m_thread_receive;
    boost::thread                           m_io_service_thread;
    hamcast::uri                            m_group;
    std::string                             m_addr;
    bool                                    m_running;
    bool                                    m_send;
    bool                                    m_receive;
    bool                                    m_service;
    std::map<std::string,boost::shared_ptr<tcp_client_connection> > m_server_connections;
    hamcast::interface_id                   m_interface;

public:
    /**
      * @brief creates a multicast mudule object
      * that periodicly sends connection request to the given multicast group
      * @param update_rate = time between the multicast requests
      * @param group = hamcast group url for the multicast group
      * @param addr = public ip address and port
      */
    multicast_module(boost::asio::io_service& io_service, int &update_rate,
                     std::string& group, std::string& addr, hamcast::interface_id ifid) :
                m_io_service(io_service), m_update_rate(update_rate),
                m_group(group), m_running(true), m_addr(addr), 
                m_send(false),m_receive(false),m_service(false),
                m_interface(ifid)
    {}

    inline hamcast::interface_id get_interface()
    {
        return m_interface;
    }
    /**
     * @brief starts the main loop for the multicast module to send connect messages
     */
    void start_send();

    /**
     * @brief starts the main loop for the multicast module to receive connect messages
     */
    void start_receive();

    /**
     * @brief to parse a connect message
     * @param msg = connect message
     */
    std::string parse_connect(std::string msg);

    /**
      * @brief cancels the main loop and waits for the thread to be terminated
      */
    void stop();


    /**
      * @brief opens a tcp connection to the given address
      */
    void connect_to_server(std::string& addr, int& port);

    inline bool& get_running(){
        return m_running;
    }

    inline hamcast::uri get_group_uri(){
        return m_group;
    }

    inline std::string get_addr(){
        return m_addr;
    }

    inline int get_update_rate(){
        return m_update_rate;
    }

    inline boost::asio::io_service& get_io_service(){
        return m_io_service;
    }
};

#endif // MULTICAST_MODULE_HPP
