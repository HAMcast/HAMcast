/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

#ifndef IGMP_LIGHT_DIS_HPP
#define IGMP_LIGHT_DIS_HPP

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <boost/thread.hpp>
#include <string>

#include "discovery/addr_storage.hpp"
#include "discovery/basic_dis.hpp"
#include "discovery/if_prop.hpp"
#include "discovery/mc_timers_values.hpp"
#include "discovery/mc_tables.hpp"

#define IGMP_LIGHT_TEST_GROUP "239.99.99.99"
#define IGMP_LIGHT_UPDATE_INTEVAL 400000 //usec
#define IGMP_LIGHT_PASSIV_TIMEOUT (MC_TV_QUERY_INTERVAL*MC_TV_ROBUSTNESS_VARIABLE) //sec ( 2*125sec)
#define IGMPV2_QUERIER_VERSISON 2

class igmp_light_dis : public basic_dis_interface{
private:
    std::string m_if_name;

     ifreq * m_item;
     if_prop* m_ifInfo;

     boost::thread* m_worker_thread;
     bool m_running;

     static void thread_worker(void *arg);

     bool m_found_protocol;
     dis_proto m_discoveredProtocol;

     bool timer_running(struct igmp_dev& dev, addr_storage& addr);
public:
    igmp_light_dis();
    ~igmp_light_dis();

    /**
     * @brief non blocking function to discover a network protocol on an spezific network interface
     * @param ifreq *item: network interface to sniff on
     *        if_prop* ifInfo: for network interface properties request
     */
    bool sniffPassiv(ifreq *item, if_prop* ifInfo);

    /**
     * @brief non blocking function to discover a network protocol on an spezific network interface
     * @param ifreq *item: network interface to sniff on
     *        if_prop* ifInfo: for network interface properties request
     */
    bool sniffActive(ifreq *item, if_prop* ifInfo);

    /**
     * @brief blocked the calling thread until the pcap sniffer has terminated
     */
    void joinSniffer();

    /**
     * @brief terminat pcap sniffer
     */
    void stopSearching();

    /**
     * @brief return true when a spezific network protocol is found
     */
    bool foundProtocol();

    /**
     * @return founded protocol
     */
    dis_proto getfoundProtocol();
};

#endif // IGMP_LIGHT_DIS_HPP
