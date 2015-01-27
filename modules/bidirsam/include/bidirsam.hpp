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

#ifndef BIDIRSAM_HPP
#define BIDIRSAM_HPP

#include <math.h>
#include <iostream>
#include <string>

#include <chimera/route.h>
#include <chimera/key.h>
#include <hamcast/uri.hpp>

//CHIMERA ROUTING TABLE
#define TABLE_ROW KEY_SIZE/BASE_B
#define TABLE_COL pow(2,BASE_B)

#define BIDIRSAM_DEFAULT_PORT 16785
#define STD_MTU_SIZE 1280

//BIDIRSAM MESSAGE CODES
#define BIDIRSAM_PREFIX_FLOODING 70
#define BIDIRSAM_JOIN_MESSAGE 71
#define BIDIRSAM_LEAVE_MESSAGE 72
#define BIDIRSAM_MULTICAST_MESSAGE 73
#define BIDIRSAM_REJOIN_MESSAGE 74

const int c_default_count = 2;          //
const int c_maintenance_timer = 60000;  // milliseconds

/*** Helper Functions and Operator Overloading ***/

inline std::string key_to_string(const Key& k)
{
    char tmp[KEY_SIZE];
    key_to_cstr(&k, tmp, KEY_SIZE);
    return std::string(tmp);
}

inline bool operator<(const Key& lhs, const Key& rhs)
{
    int less = key_comp (&lhs,&rhs);
    if (less < 0)
    {
        return true;
    }
    return false;
    //return (key_comp (&lhs,&rhs)<0);
}

inline bool operator==(const Key& lhs, const Key& rhs)
{
    int equal = key_equal (lhs, rhs);
    if (equal == 1)
    {
        return true;
    }
    return false;
    //return (key_comp (&lhs,&rhs)==0);
}

#endif // BIDIRSAM_HPP

