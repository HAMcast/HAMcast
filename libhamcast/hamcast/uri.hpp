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

#ifndef HAMCAST_URI_HPP
#define HAMCAST_URI_HPP

#include <string>
#include <ostream>

#include <boost/cstdint.hpp>

#include "hamcast/intrusive_ptr.hpp"
#include "hamcast/util/comparable.hpp"

namespace hamcast {

namespace detail { class uri_private; }

/**
 * @brief Uniform Resource Identifier (as defined in RFC 3986).
 * @include uri_documentation.txt
 * @note The documentation of the accessors are taken from RFC 3986.
 */
class uri : util::comparable<uri, uri>, util::comparable<uri, const char*>
          , util::comparable<uri, std::string>
{

 public:

    struct add_ref { void operator()(detail::uri_private*); };
    struct release { void operator()(detail::uri_private*); };

    // required by util::comparable<uri, uri>
    inline int compare(const uri& what) const
    {
        return (this == &what) ? 0 : str().compare(what.str());
    }

    // required by util::comparable<uri, std::string>
    inline int compare(const std::string& what) const
    {
        return str().compare(what);
    }

    // required by util::comparable<uri, const char*>
    inline int compare(const char* what) const
    {
        // treat a NULL string like an empty string
        return (what) ? str().compare(what) : str().compare("");
    }

    /**
     * @brief Create an empty URI.
     */
    uri();

    /**
     * @brief Create an URI from @p uri_str.
     *
     * If @p uri_str could not be parsed to a valid URI, then this URI
     * object will be empty.
     *
     * @param uri_str An URI encoded as a string.
     *
     * @warning Let <code>a</code> be a string then the assertion
     *          <code>a.empty() == uri(a).empty()</code> fails, if
     *          <code>a</code> is not empty, but does not describe a valid URI
     *          (e.g. "Hello World" is a string, but is an invalid URI).
     */
    uri(const std::string& uri_str);

    /**
     * @brief Create an URI from @p uri_str_c_str.
     * @param uri_c_str An URI encoded as a C-string.
     * @see uri(const std::string&)
     */
    uri(const char* uri_c_str);

    /**
     * @brief Create this object as a copy of @p other.
     * @param other The original {@link hamcast::uri uri} object.
     * @note {@link hamcast::uri uri} is implicit shared, thus copy
     *       operations are very fast and lightweight.
     */
    uri(const uri& other);

    /**
     * @brief Get the string describing this URI.
     * @returns The full string representation of this URI.
     */
    const std::string& str() const;

    /**
     * @brief Get the string describing this uri as a C-string.
     * @returns The full string representation of this URI as C-string.
     * @note Equal to <code>str().c_str()</code>.
     */
    inline const char* c_str() const { return str().c_str(); }

    /**
     * @brief Check if this URI is empty.
     * @returns <code>true</code> if this URI is empty;
     *         otherwise <code>false</code>.
     */
    bool empty() const { return str().empty(); }

    /**
     * @brief Get the host subcomponent of authority.
     *
     * The host subcomponent of authority is identified by an IP literal
     * encapsulated within square brackets, an IPv4 address in dotted-
     * decimal form, or a registered name.
     *
     * @returns The host subcomponent of {@link authority()}.
     */
    const std::string& host() const;

    /**
     * @brief Check if {@link host()} returns an IPv4 address.
     * @note The testing is done in the constructor, so this member
     *       function only checks an internal flag (and has no
     *       other overhead!).
     * @returns <code>true</code> if the host subcomponent of {@link authority()}
     *         returns a string that describes a valid IPv4 address;
     *         otherwise <code>false</code>.
     */
    bool host_is_ipv4addr() const;

    /**
     * @brief Check if {@link host()} returns an IPv6 address.
     * @note Returns true if host matches the regex
     *       <code>[a-f0-9:\\.]</code> so {@link host()} might be an
     *       invalid ipv6 address.
     * @note The testing is done in the constructor, so this member
     *       function only checks an internal flag (and has no
     *       other overhead!).
     * @returns <code>true</code> if the host subcomponent of {@link authority()}
     *         returns a string that describes an IPv6 address;
     *         otherwise <code>false</code>.
     * @warning This member function does not guarantee, that {@link host()}
     *          returns a <b>valid</b> IPv6 address.
     */
    bool host_is_ipv6addr() const;

    /**
     * @brief Get the port subcomponent of authority.
     *
     * Port is either empty or a decimal number (between 0 and 65536).
     * @returns A string representation of the port
     *         subcomponent of {@link authority()}.
     */
    const std::string& port() const;

    /**
     * @brief Get the port subcomponent as integer value.
     *
     * This value is always 0 if <code>port().empty() == true</code>.
     * @returns An integer (16-bit, unsigned) representation of the port
     *         subcomponent of {@link authority()}.
     */
    boost::uint16_t port_as_int() const;

    /**
     * @brief Get the user information subcomponent of authority.
     *
     * The userinfo subcomponent may consist of a user name and, optionally,
     * scheme-specific information about how to gain authorization to access
     * the resource.
     * @returns The user information subcomponent of {@link authority()}.
     */
    const std::string& user_information() const;

    /**
     * @brief Get the user information plus host subcomponents of authority.
     */
    const std::string& user_information_and_host() const;

    /**
     * @brief Get the path component of this URI object.
     *
     * The path component contains data that serves to identifiy a resource
     * within the scope of the URI's scheme and naming authority (if any).
     * @returns The path component.
     */
    const std::string& path() const;

    /**
     * @brief Get the query component of this URI object.
     *
     * The query component contains non-hierarchical data that, along with
     * data in the path component (Section 3.3), serves to identify a
     * resource within the scope of the URI's scheme and naming authority
     * (if any).
     * @returns The query component.
     */
    const std::string& query() const;

    /**
     * @brief Get the scheme component of this URI object.
     *
     * Each URI begins with a scheme name that refers to a specification for
     * assigning identifiers within that scheme.
     * @returns The scheme component.
     */
    const std::string& scheme() const;

    /**
     * @brief Get the fragment component of this URI object.
     *
     * The fragment identifier component of a URI allows indirect
     * identification of a secondary resource by reference to a primary
     * resource and additional identifying information.
     * @returns The fragment component.
     */
    const std::string& fragment() const;

    /**
     * @brief Get the authority component of this URI object.
     *
     * The subcomponents of authority could be queried with
     * {@link user_information()}, {@link host()} and {@link port()}.
     * @returns The authority component.
     */
    const std::string& authority() const;

    /**
     * @brief Exchanges the contents of <code>this</code> and @p other.
     * @param other {@link hamcast::uri uri} object that should exchange its
     *              content with the content of <code>this</code>.
     */
    void swap(uri& other);

    /**
     * @brief Equivalent to <code>uri(other).swap(*this)</code>.
     * @param other Original {@link hamcast::uri uri} object.
     * @returns <code>*this</code>.
     */
    uri& operator=(const uri& other);

 private:

    intrusive_ptr<detail::uri_private, add_ref, release> d;

};

} // namespace hamcast

// enable serialization / deserialization
namespace hamcast { namespace util {

class serializer;
class deserializer;

serializer& operator<<(serializer& s, const uri& what);
deserializer& operator>>(deserializer& d, uri& storage);

} } // namespace hamcast::util

namespace std
{

inline ostream& operator<<(ostream& ostr, const hamcast::uri& what)
{
    return (ostr << what.str());
}

}

#endif // HAMCAST_URI_HPP
