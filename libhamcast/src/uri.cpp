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

boost::regex m_authority_rx("(?:([a-zA-Z0-9:_\\.]+)@)?"
                            "([a-zA-Z0-9_\\.]+|\\[[a-fA-F0-9:\\.]+])"
                            "(?:[:]([0-9]+))?");

boost::regex m_v4_rx(HC_OCTET HC_DOT HC_OCTET HC_DOT HC_OCTET HC_DOT HC_OCTET);

} // namespace <anonymous>

//    foo://example.com:8042/over/there?name=ferret#nose
//    \_/   \______________/\_________/ \_________/ \__/
//     |           |            |            |        |
//  scheme     authority       path        query   fragment
//     |   _____________________|__
//    / \ /                        \.
//    urn:example:animal:ferret:nose

namespace hamcast { namespace detail {

class uri_private : public ref_counted
{

    // enum values are commented with transitions (char -> next state)
    // each "state" has the implicit transition "{default} -> self"
    enum
    {
        // ":" -> parse_auth_or_path
        //  $  -> error
        parse_scheme,
        // "[^/]" -> parse_path
        //  "/"   -> parse_auth_or_path_first_slash_read
        parse_auth_or_path,
        // "[^/]" -> parse_path
        //  "/"   -> parse_auth
        parse_auth_or_path_first_slash_read,
        // "/" -> parse_path
        // "?" -> parse_query
        // "#" -> parse_fragment
        parse_auth,
        // "?" -> parse_query
        // "#" -> parse_fragment
        parse_path,
        // "#" -> parse_fragment
        parse_query,
        // no transitions
        parse_fragment,
        // no transitions
        parse_error
    }
    m_state;

    enum
    {
        default_flag,
        ipv4_flag,
        ipv6_flag
    }
    m_flag;

    // uri components
    std::string m_uri;
    std::string m_path;
    std::string m_query;
    std::string m_scheme;
    std::string m_fragment;
    std::string m_authority;

    // authority subcomponents
    std::string m_user_information;
    std::string m_host;
    std::string m_user_information_and_host;
    std::string m_port;

    // convenience fields
    boost::uint16_t m_int_port;

    void clear()
    {
        m_uri.clear();
        m_path.clear();
        m_query.clear();
        m_scheme.clear();
        m_fragment.clear();
        m_authority.clear();
    }

    bool consume(char c)
    {
        switch (m_state)
        {

         case parse_scheme:
            if (c == ':') m_state = parse_auth_or_path;
            else m_scheme += c;
            return true;

         case parse_auth_or_path:
            if (c == '/') m_state = parse_auth_or_path_first_slash_read;
            else
            {
                m_state = parse_path;
                m_path += c;
            }
            return true;

         case parse_auth_or_path_first_slash_read:
            if (c == '/') m_state = parse_auth;
            else
            {
                m_state = parse_path;
                m_path += '/';
                m_path += c;
            }
            return true;

         case parse_auth:
            switch (c)
            {

             case '/':
                m_state = parse_path;
                m_path += '/';
                break;

             case '?':
                m_state = parse_query;
                break;

             case '#':
                m_state = parse_fragment;
                break;

             default: m_authority += c;

            }
            return true;

         case parse_path:
            if (c == '?') m_state = parse_query;
            else if (c == '#') m_state = parse_fragment;
            else m_path += c;
            return true;

         case parse_query:
            if (c == '#') m_state = parse_fragment;
            else m_query += c;
            return true;

         case parse_fragment:
            m_fragment += c;
            return true;

         default:
            m_state = parse_error;
            return false;

        }
    }

    // this parses the given uri to the form
    // {scheme} {authority} {path} {query} {fragment}
    bool parse_uri(const std::string& what)
    {
        m_state = parse_scheme;
        clear();
        m_uri = what;
        for (size_t i = 0; i < m_uri.size(); ++i)
        {
            if (!consume(m_uri[i])) return false;
        }
        return m_state != parse_scheme && m_state != parse_error;
    }

    // parse $what with parse_uri and then parse $authority
    bool parse(const std::string& what)
    {
        boost::smatch sm;
        m_flag = default_flag;
        if (parse_uri(what)
            && (m_authority.empty()
                || boost::regex_match(m_authority, sm, m_authority_rx)))
        {
            if (!m_authority.empty())
            {
                m_user_information = sm[1];
                m_host = sm[2];
                if (!m_host.empty() && m_host[0] == '[')
                {
                    // ipv6 address
                    m_flag = ipv6_flag;
                    // erase leading "[" and trailing "]"
                    m_host = m_host.substr(1, m_host.size() - 2);
                }
                else if (!m_host.empty() && boost::regex_match(m_host, m_v4_rx))
                {
                    m_flag = ipv4_flag;
                }
                m_port = sm[3];
                if (m_port.empty()) m_int_port = 0;
                else
                {
                    m_int_port =
                            static_cast<boost::uint16_t>(atoi(m_port.c_str()));
                }
            }
            if (m_user_information.empty())
            {
                m_user_information_and_host = m_host;
            }
            else
            {
                m_user_information_and_host  = m_user_information;
                m_user_information_and_host += "@";
                m_user_information_and_host += m_host;
            }
            return true;
        }
        else return false;
    }

public:

    inline uri_private() { }

    static uri_private* from(const std::string& what);

    static uri_private* from(const char* what);

    inline const std::string& path() const { return m_path; }

    inline const std::string& query() const { return m_query; }

    inline const std::string& scheme() const { return m_scheme; }

    inline const std::string& fragment() const { return m_fragment; }

    inline const std::string& authority() const { return m_authority; }

    inline const std::string& as_string() const { return m_uri; }

    inline const std::string& host() const { return m_host; }

    inline const std::string& port() const { return m_port; }

    inline boost::uint16_t port_as_int() const { return m_int_port; }

    inline const std::string& user_information_and_host() const
    {
        return m_user_information_and_host;
    }

    inline const std::string& user_information() const
    {
        return m_user_information;
    }

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

const std::string& uri::host() const
{
    return d->host();
}

const std::string& uri::port() const
{
    return d->port();
}

boost::uint16_t uri::port_as_int() const
{
    return d->port_as_int();
}

const std::string& uri::user_information() const
{
    return d->user_information();
}

const std::string& uri::path() const
{
    return d->path();
}

const std::string& uri::query() const
{
    return d->query();
}

const std::string& uri::scheme() const
{
    return d->scheme();
}

const std::string& uri::fragment() const
{
    return d->fragment();
}

const std::string& uri::authority() const
{
    return d->authority();
}

const std::string& uri::user_information_and_host() const
{
    return d->user_information_and_host();
}

bool uri::host_is_ipv4addr() const
{
    return d->host_is_ipv4addr();
}

bool uri::host_is_ipv6addr() const
{
    return d->host_is_ipv6addr();
}

void uri::add_ref::operator()(detail::uri_private* rc)
{
    ref_counted::add_ref(rc);
}

void uri::release::operator()(detail::uri_private* rc)
{
    ref_counted::release(rc);
}

} // namespace hamcast

/*
void test_uri_parser()
{
    const char* uris[] = {
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        "http://www.ietf.org/rfc/rfc2396.txt",
        "ldap://[2001:db8::7]/c=GB?objectClass?one",
        "mailto:John.Doe@example.com",
        "news:comp.infosystems.www.servers.unix",
        "tel:+1-816-555-1212",
        "telnet://192.0.2.16:80/",
        "urn:oasis:names:specification:docbook:dtd:xml:4.1.2",
        "foo://example.com:8042/over/there?name=ferret#nose",
        0
    };
}
*/
