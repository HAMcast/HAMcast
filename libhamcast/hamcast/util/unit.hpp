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

#ifndef HAMCAST_UTIL_UNIT_HPP
#define HAMCAST_UTIL_UNIT_HPP

#include <boost/type_traits/integral_constant.hpp>

namespace hamcast { namespace util {

/**
 * @brief Marker class used in templates as "void" type.
 */
struct unit { };

template<typename T>
struct is_unit : boost::false_type { };

template<>
struct is_unit<unit> : boost::true_type { };

template<typename T>
struct is_not_unit : boost::true_type { };

template<>
struct is_not_unit<unit> : boost::false_type { };

// Get the position of the first unit.
// e.g.:
//		first_unit<int>::value == 1
//		first_unit<int, double>::value == 2
//		first_unit<unit>::value == first_unit<>::value == 0
template<typename T1 = unit, typename T2 = unit, typename T3 = unit,
         typename T4 = unit, typename T5 = unit, typename T6 = unit>
struct first_unit
{
    static const int value = 1 + first_unit<T2,T3,T4,T5,T6>::value;
};

template<typename T2, typename T3, typename T4, typename T5, typename T6>
struct first_unit<unit,T2,T3,T4,T5,T6>
{
    static const int value = 0;
};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_UNIT_HPP
