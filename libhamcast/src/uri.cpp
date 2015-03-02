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

#include <string>
#include <cctype>
#include <iostream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

#include "hamcast/uri.hpp"
#include "hamcast/ref_counted.hpp"
#include "hamcast/intrusive_ptr.hpp"

// allows 0-255, but not 00 for example
#define HC_OCTET "(?:[1-9][0-9]|[0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])"
#define HC_DOT "\\."

namespace {

boost::regex m_v4_rx(HC_OCTET HC_DOT HC_OCTET HC_DOT HC_OCTET HC_DOT HC_OCTET);

} // namespace <anonymous>


//  ham-URI = ham-scheme ":" namespace ":" group [ "@" instantiation ]
//                                 [ ":" port ] [ "/" sec-credentials ]
//      
//        ham:namespace:group.com@instantiation:1234/sec-credentials
//        \_/ \_______/ \_______/ \___________/ \__/ \_____________/
//         |      |           |         |         |         |
// ham_scheme  ham_namespace  |    instantiation  | sec_credentials
//                          group                port   

// e.g. ham:namespace:group
// e.g. ham:namespace:group@instantiation
// e.g. ham:namespace:group@instantiation:1234
// e.g. ham:namespace:group@instantiation:1234/sec-credentials
//  
// e.g. ham:namespace:group:1234
// e.g. ham:namespace:group/sec-credentials

namespace hamcast { namespace detail {

class uri_private : public ref_counted
{

    // enum values are commented with transitions (char -> next state)
    // each "state" has the implicit transition "{default} -> self"
    enum
    {
        parse_ham_scheme,
        parse_ham_namespace,
        parse_group,
        parse_instantiation,
        parse_port,
        parse_sec_credentials,
        parse_error
    }
    m_state;

    bool m_waitfor_square_bracket;

    enum
    {
        default_flag,
        ipv4_flag,
        ipv6_flag
    }
    m_flag;

    std::string m_ham_scheme;
    std::string m_ham_namespace;
    std::string m_group;
    std::string m_instantiation;
    std::string m_port;
    std::string m_sec_credentials;
    
    // uri components
    std::string m_uri;

     //convenience fields
    boost::uint16_t m_int_port;

    void clear()
    {
        m_ham_scheme.clear();
        m_ham_namespace.clear();
        m_group.clear();
        m_instantiation.clear();
        m_port.clear();
        m_sec_credentials.clear();
    }

    bool consume(char c)
    {
        switch (m_state)
        {

         case parse_ham_scheme:
            if (c == ':') m_state = parse_ham_namespace;
            else m_ham_scheme += c;
            return true;

         case parse_ham_namespace:
            if (c == ':') m_state = parse_group;
            else m_ham_namespace += c;
            return true;

         case parse_group:
            if(!m_waitfor_square_bracket)
            {
                if (c == '@')
                {
                    m_state = parse_instantiation;
                    return true;
                }
                else if (c == ':')
                {
                    m_state = parse_port;
                    return true;
                }
                else if (c == '/')
                {
                    m_state = parse_sec_credentials;
                    return true;
                }
                else if (c == '[')
                {
                    m_waitfor_square_bracket = true;
                }
            }
            
            if (c == ']')
            {
                m_waitfor_square_bracket = false;
            }
            
            m_group += c;
            return true;

         case parse_instantiation:
            if(!m_waitfor_square_bracket)
            {
                if (c == ':')
                {
                    m_state = parse_port;
                    return true;
                }
                else if (c == '/')
                {
                    m_state = parse_sec_credentials;
                    return true;
                }
                else if (c == '[')
                {
                    m_waitfor_square_bracket = true;                
                }
            }
            
            if (c == ']')
            {
                m_waitfor_square_bracket = false; 
            }
            
            m_instantiation += c;
            return true;

         case parse_port:
            if (c == '/') m_state = parse_sec_credentials;
            else m_port += c;
            return true;

         case parse_sec_credentials:
            m_sec_credentials += c;
            return true;

         default:
            m_state = parse_error;
            return false;

        }
    }

    // this parses the given uri 
    bool parse_uri(const std::string& what)
    {
        m_state = parse_ham_scheme;
        m_waitfor_square_bracket = false;
        clear();
        for (size_t i = 0; i < what.size(); ++i)
        {
            char c = std::tolower(what[i]);
            m_uri += c;
            if (!consume(c)) return false;
        }
    
        if(m_ham_scheme.compare("ham") == 0
               && !m_ham_namespace.empty()
               && !m_group.empty()
               && !m_waitfor_square_bracket)
        {
            return true;
        }
        else
        {
            clear(); 
            return false;
        }
    }

    // parse $what with parse_uri and then parse $authority
    bool parse(const std::string& what)
    {
        boost::smatch sm;
        m_flag = default_flag;
        if (parse_uri(what))
        {
            if (!m_group.empty() && m_group[0] == '[')
            {
                // ipv6 address
                m_flag = ipv6_flag;
                // erase leading "[" and trailing "]"
                m_group = m_group.substr(1, m_group.size() - 2);
            }
            else if (!m_group.empty() && boost::regex_match(m_group, m_v4_rx))
            {
                m_flag = ipv4_flag;
            }
           
            if (!m_instantiation.empty() && m_instantiation[0] == '[')
            {
                m_instantiation = m_instantiation.substr(1, m_instantiation.size() - 2);
            }

            if (m_port.empty()) m_int_port = 0;
            else
            {
                m_int_port = static_cast<boost::uint16_t>(atoi(m_port.c_str()));
            }
            return true;
        }
        else return false;
    }

public:

    inline uri_private() { }

    static uri_private* from(const std::string& what);

    static uri_private* from(const char* what);
    
    inline const std::string& ham_scheme() const { return m_ham_scheme; }

    inline const std::string& ham_namespace() const { return m_ham_namespace; }

    inline const std::string& group() const { return m_group; }

    inline const std::string& instantiation() const { return m_instantiation; }

    inline const std::string& port() const { return m_port; }

    inline const std::string& sec_credentials() const { return m_sec_credentials; }

    inline const std::string& as_string() const { return m_uri; }

    inline boost::uint16_t port_as_int() const { return m_int_port; }

    inline bool host_is_ipv4addr() const
    {
        return m_flag == ipv4_flag;
    }

    inline bool host_is_ipv6addr() const
    {
        return m_flag == ipv6_flag;
    }

};

namespace {

intrusive_ptr<uri_private, uri::add_ref, uri::release> m_default_uri_private(new hamcast::detail::uri_private);

} // namespace <anonymous>

uri_private* uri_private::from(const std::string& what)
{
    uri_private* result = new uri_private;
    if (result->parse(what)) return result;
    else
    {
        delete result;
        return m_default_uri_private.get();
    }
}

uri_private* uri_private::from(const char* what)
{
    std::string tmp(what);
    return from(tmp);
}

} } // namespace hamcast::detail

namespace hamcast {

uri::uri() : d(detail::m_default_uri_private)
{
}

uri::uri(const std::string& uri_str) : d(detail::uri_private::from(uri_str))
{
}

uri::uri(const char* cstr) : d(detail::uri_private::from(cstr))
{
}

uri::uri(const uri& other) : d(other.d)
{
}

uri& uri::operator=(const uri& other)
{
    d = other.d;
    return *this;
}

const std::string& uri::str() const
{
    return d->as_string();
}

const std::string& uri::port() const
{
    return d->port();
}

boost::uint16_t uri::port_as_int() const
{
    return d->port_as_int();
}

bool uri::host_is_ipv4addr() const
{
    return d->host_is_ipv4addr();
}

bool uri::host_is_ipv6addr() const
{
    return d->host_is_ipv6addr();
}

const std::string& uri::ham_scheme() const
{
    return d->ham_scheme();
}

const std::string& uri::ham_namespace() const
{
    return d->ham_namespace();    
}

const std::string& uri::group() const
{
    return d->group();
}

const std::string& uri::instantiation() const
{
    return d->instantiation();
}

const std::string& uri::sec_credentials() const
{
    return d->sec_credentials();
}


void uri::add_ref::operator()(detail::uri_private* rc)
{
    ref_counted::add_ref(rc);
}

void uri::release::operator()(detail::uri_private* rc)
{
    ref_counted::release(rc);
}


//void print_uri(const uri& u)
//{
    //std::cout << u << std::endl;
    //std::cout << "\tham_scheme:" << u.ham_scheme() << std::endl;
    //std::cout << "\tham_namespace: " << u.ham_namespace() << std::endl;
    //std::cout << "\thost_is_ipv4addr: " << u.host_is_ipv4addr() << std::endl;
    //std::cout << "\thost_is_ipv6addr: " << u.host_is_ipv6addr() << std::endl;
    //std::cout << "\tport: " << u.port() << std::endl;
    //std::cout << "\tport_as_int: " << u.port_as_int() << std::endl;
    //std::cout << "\tgroup: " << u.group() << std::endl;
    //std::cout << "\tinstantiation: " << u.instantiation() << std::endl;
    //std::cout << "\tsec_credentials: " << u.sec_credentials() << std::endl;
//}

//void uri::test_uri()
//{
    //const char* uris1[] = {
        //"ham:namespace:group.com",
        //"ham:namespace:*",
        //"ham:namespace:group.com@instantiation",
        //"ham:namespace:*@instantiation",
        //"ham:namespace:group.com@instantiation:1234",
        //"ham:namespace:group.com@instantiation:1234/seccredentials",
        //"ham:namespace:*@instantiation:1234/seccredentials",
        //"ham:namespace:GROUP.com@instantiAtIon:1234/SECCREDENTIALS",
        //"ham:ip:224.1.2.3:5000",
        //"ham:sip:news@cnn.com",
        //"ham:opaque:news@cnn.com",
        //"ham:ip:[2::2:2]:5000",
        //"ham:ip:[123::22]:5000",
        //"ham:ip:[123::22]@[123::99]:33"
    //};
  ////ham-URI   = ham-scheme ":" namespace ":" group [ "@" instantiation ]
      ////[ ":" port ] [ "/" sec-credentials ]
    //for(unsigned int i= 0; i < sizeof(uris1)/sizeof(char*); ++i){
        //std::cout << uris1[i] << std::endl;
        //print_uri(uri(uris1[i]));
    //}
//}

} // namespace hamcast

