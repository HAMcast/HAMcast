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

#ifndef HAMCAST_UTIL_COMPARABLE_HPP
#define HAMCAST_UTIL_COMPARABLE_HPP

namespace hamcast { namespace util {

/**
 * @brief Barton-Nackman trick for all comparsion operators.
 *
 * @p Super must implement an "int compare(const T& what)" member function
 *    that return 0 if <code>this == what</code>, -1 if
 *    <code>this < what</code> and 1 if <code>this > what</code>.
 */
template<class Super, typename T>
class comparable
{
    friend bool operator==(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator==(const T& lhs, const Super& rhs)
    {
        return rhs.compare(lhs) == 0;
    }
    friend bool operator!=(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) != 0;
    }
    friend bool operator!=(const T& lhs, const Super& rhs)
    {
        return rhs.compare(lhs) != 0;
    }
    friend bool operator<(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator<(const T& lhs, const Super& rhs)
    {
        // lhs < rhs <=> rhs > lhs
        return rhs.compare(lhs) > 0;
    }
    friend bool operator>(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator>(const T& lhs, const Super& rhs)
    {
        // lhs > rhs <=> rhs < lhs
        return rhs.compare(lhs) < 0;
    }
    friend bool operator<=(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) <= 0;
    }
    friend bool operator<=(const T& lhs, const Super& rhs)
    {
        // lhs <= rhs <=> rhs >= lhs
        return rhs.compare(lhs) >= 0;
    }
    friend bool operator>=(const Super& lhs, const T& rhs)
    {
        return lhs.compare(rhs) >= 0;
    }
    friend bool operator>=(const T& lhs, const Super& rhs)
    {
        // lhs >= rhs <=> rhs <= lhs
        return rhs.compare(lhs) <= 0;
    }
};

template<class Super>
class comparable<Super, Super>
{
    friend bool operator==(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) == 0;
    }
    friend bool operator!=(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) != 0;
    }
    friend bool operator<(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) < 0;
    }
    friend bool operator>(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) > 0;
    }
    friend bool operator<=(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) <= 0;
    }
    friend bool operator>=(const Super& lhs, const Super& rhs)
    {
        return lhs.compare(rhs) >= 0;
    }
};

} } // namespace hamcast::util

#endif // HAMCAST_UTIL_COMPARABLE_HPP
