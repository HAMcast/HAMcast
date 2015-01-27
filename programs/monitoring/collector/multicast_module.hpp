#ifndef MULTICAST_MODULE_HPP
#define MULTICAST_MODULE_HPP

#include <string>

#include <boost/thread.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "hamcast/hamcast.hpp"
#include "http_message.hpp"


class multicast_module
{

private:
    int m_update_rate;
    boost::thread m_thread_send;
    boost::thread m_thread_receive;
    hamcast::uri m_group;
    std::string m_addr;
    bool m_running;
    bool m_send;
    bool m_receive;



public:
    /**
      * @brief creates a multicast mudule object
      * that periodicly sends connection request to the given multicast group
      * @param update_rate = time between the multicast requests
      * @param group = hamcast group url for the multicast group
      * @param addr = public ip address and port
      */
    multicast_module(int& update_rate, std::string& group, std::string& addr);
    /**
     * @brief starts the main loop for the multicast module
     */
    void start_send();

    void start_receive();

    /**
      * @brief creates connect message
      */
    std::string create_connect();
    /**
      * @brief cancels the main loop and waits for the thread to be terminated
      */
    void stop();

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
};

#endif // MULTICAST_MODULE_HPP
