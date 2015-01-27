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

#ifndef ADDR_STORAGE_HPP
#define ADDR_STORAGE_HPP

#include <iostream>
#include <sys/socket.h>
#include <string>

#define INIT_ADDR_FAMILY -1
class addr_storage
{
private:
     struct sockaddr_storage m_addr;
public:
    addr_storage();
    addr_storage(int addr_family);
    addr_storage(const std::string& m_addr);
    addr_storage(const struct sockaddr_storage& m_addr);
    addr_storage(const addr_storage& m_addr);
    addr_storage(const struct in_addr& m_addr);
    addr_storage(const struct in6_addr& m_addr);
    addr_storage(const struct sockaddr& m_addr);

    /**
     * @brief default copy operator
     */
    addr_storage& operator=(const addr_storage& s);

    /**
     * @brief copy operator struct sockaddr_storage to class addr_storage
     */
    addr_storage& operator=(const struct sockaddr_storage& s);

    /**
     * @brief copy operator string to class addr_storage
     */
    addr_storage& operator=(const std::string& s);

    /**
     * @brief copy operator struct in_addr to class addr_storage
     */
    addr_storage& operator=(const struct in_addr& s);

    /**
     * @brief copy operator struct in6_addr to class addr_storage
     */
    addr_storage& operator=(const struct in6_addr& s);

    /**
     * @brief copy operator struct sockaddr to class addr_storage
     */
    addr_storage& operator=(const struct sockaddr& s);

    /**
     * @brief compare two addresses if one of this addresses unknown the function returns false
     */
    bool operator==(addr_storage& addr);

    /**
     * @brief disjunction to operator==
     */
    bool operator!=(addr_storage& addr);

    /**
     * @return struct sockaddr_storage
     */
    struct sockaddr_storage get_sockaddr_storage();

    /**
     * @return current address family AF_INET or AF_INET6 or INIT_ADDR_FAMILY
     */
    int get_addr_family();

    /**
     * @return current address as string or "??" for unknown address
     */
    std::string to_string();

    /**
     * @brief simple test output
     */
    static void test_addr_storage();

    /**
     * @brief cout output operator
     */
    friend std::ostream& operator <<(std::ostream& s, const addr_storage a);

    /**
     * @brief copy operator "<<=" class addr_storage& to struct sockaddr_storage
     */
    friend struct sockaddr_storage& operator<<=(struct sockaddr_storage& l,const addr_storage& r);

    /**
     * @brief copy operator "<<=" class addr_storage& to struct in_addr
     */
    friend struct in_addr& operator<<=(struct in_addr& l,const addr_storage& r);

    /**
     * @brief copy operator "<<=" class addr_storage& to struct in6_addr
     */
    friend struct in6_addr& operator<<=(struct in6_addr& l,const addr_storage& r);
};



#endif // ADDR_STORAGE_HPP

