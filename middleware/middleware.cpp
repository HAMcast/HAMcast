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

#include "middleware.hpp"

namespace hamcast { namespace middleware {

std::vector<uri_mapping> possible_mappings(const hamcast::uri& group)
{
    std::vector<uri_mapping> result;
    std::vector<tech_module::ptr> mods = tech_module::instances();
    for (std::vector<tech_module::ptr>::iterator i(mods.begin());
         i != mods.end();
         ++i)
    {
        tech_module& tm = *(i->get());
        hamcast::uri u = tm.mapping_of(group);
        if (!u.empty()) result.push_back(uri_mapping(u, *i));
    }
    {
        std::string msg;
        msg = "Possible mappings of ";
        msg += group.str();
        msg += " (namespace = ";
        msg += group.ham_namespace();
        msg += ") { ";
        for (size_t i = 0; i < result.size(); ++i)
        {
            if (i > 0) msg += ", ";
            msg += result[i].first.str();
        }
        msg += " }";
        HC_LOG_DEBUG(msg);
    }
    return result;
}

uri_mapping select_mapping(const hamcast::uri&,
                           const std::vector<uri_mapping>& choices)
{
    //HC_LOG_TRACE("choices.size() = " << choices.size());
    if (choices.empty())
    {
        HC_LOG_ERROR("choices.size() == 0");
        throw std::logic_error("choices.size() == 0");
    }
    // todo ...
    return choices.front();
}

} } // namespace hamcast::middleware
