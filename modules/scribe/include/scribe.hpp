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

#ifndef _SCRIBE_HPP_
#define _SCRIBE_HPP_

/**
  * @author Sebastian Meiling <sebastian.meiling (at) haw-hamburg.de>
  */

/**
  * @def SCRIBE_DEFAULT_PORT Default port for scribe/chimera overlay, if not set in middleware.ini
  * @def SCRIBE_DEFAULT_MTU Maximum transmission unit in bytes
  * @def SCRIBE_MSG_ABC Several scribe message types
  */

#include <string>

#include "chimera/key.h"

/* scribe misc parameters */
#define SCRIBE_DEFAULT_PORT 16783       // chimera port
#define SCRIBE_DEFAULT_MTU 1280         //

/* scribe standard message types */
#define SCRIBE_MSG_UNKNOWN    50        // reserved
#define SCRIBE_MSG_CREATE     51        // group create
#define SCRIBE_MSG_JOIN       52        // group join
#define SCRIBE_MSG_LEAVE      53        // group leave
#define SCRIBE_MSG_MULTICAST  54        // group multicast

/* scribe control message types */
#define SCRIBE_MSG_PING       60        // some ping
#define SCRIBE_MSG_HEARTBEAT  61        // group heartbeat
#define SCRIBE_MSG_RP_REQUEST 62        // request group rp
#define SCRIBE_MSG_RP_REPLY   63        // reply with group rp
#define SCRIBE_MSG_REPLICATE  64        // send and replicate node state
#define SCRIBE_MSG_PARENT     65        // send parent infos

namespace scribe {

const int c_default_count = 2;          // join/leave soft state robustness variable
const int c_maintenance_timer = 60000;  // milliseconds


} /* namespace scribe */

/**
 * @brief operator <
 * @param lhs
 * @param rhs
 * @return
 */
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

/**
 * @brief operator ==
 * @param lhs
 * @param rhs
 * @return
 */
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

/**
 * @brief key_to_string
 * @param k
 * @return
 */
inline std::string key_to_string(Key& k)
{
    char tmp[KEY_SIZE];
    key_to_cstr(&k, tmp, KEY_SIZE);
    return std::string(tmp);
}

#endif /* _SCRIBE_HPP_ */
