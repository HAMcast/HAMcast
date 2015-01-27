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

#include <limits>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "hamcast/hamcast.hpp"

using namespace hamcast;

int main(int argc, char **argv)
{
    if (argc == 3) {
        hamcast::uri group (argv[1]);
        int iface = atoi(argv[2]);
        std::vector<uri> parents = parent_set(iface, group);
        std::vector<interface_property> ifs = get_interfaces();
        for (size_t l=0; l < ifs.size(); ++l) {
            if (ifs[l].id == iface) {
                std::cout << "iface[" << ifs[l].id << "] " << ifs[l].address << std::endl; 
            }
        }
        ifs = get_interfaces();
        for (size_t m=0; m < parents.size(); ++m) {
            std::cout << "parent[" << m << "] " << parents[m] << std::endl;
        }
    }
    else {
        std::cout << "USAGE: ./getparent <group URI> <interface ID> " << std::endl;
    }
    return 0;
}

