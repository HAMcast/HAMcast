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

#ifndef MLD_LIGHT_DIS_HPP
#define MLD_LIGHT_DIS_HPP

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string>
#include <cstring>
#include <boost/thread.hpp>



#include "discovery/basic_dis.hpp"
#include "discovery/if_prop.hpp"
#include "discovery/mc_tables.hpp"
#include "discovery/mc_timers_values.hpp"

#define MLD_LIGHT_PASSIV_TIMEOUT (MC_TV_QUERY_INTERVAL*MC_TV_ROBUSTNESS_VARIABLE) //sec ( 2*125sec)
#define MLD_LIGHT_EXIT_TIME 1


class mld_light_dis : public basic_dis_interface{
private:
     mc_tables m_t6;
     std::string m_if_name;
     struct snmp6 m_snmp_infos_old;

     ifreq * m_item;
     if_prop* m_ifInfo;

     boost::thread* m_worker_thread;
     bool m_running;

     static void thread_worker(void *arg);

     bool m_found_protocol;
     dis_proto m_discoveredProtocol;
protected:
public:
    mld_light_dis();
    ~mld_light_dis();

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

#endif // MLD_LIGHT_DIS_HPP
