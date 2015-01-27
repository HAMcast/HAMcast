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

#ifndef HAMCAST_EXCEPTIONS_HPP
#define HAMCAST_EXCEPTIONS_HPP

#include <exception>
#include <stdexcept>

#include "hamcast/hamcast_logging.h"

namespace hamcast {

/**
 * @brief Describes all possible errors that can occur if you try
 *        to establish a connection to the middleware.
 * @ingroup ExceptionHandling
 */
enum connection_error
{
    /**
     * @brief Indicates that no config file of the middleware was found.
     *
     * This usually means that no middleware is running.
     */
    no_config_file_found,
    /**
     * @brief Indicates that there is currently no middleware is running.
     */
    no_running_middleware_found,
    /**
     * @brief Indicates that a socket creation failed.
     */
    socket_creation_failed,
    /**
     * @brief Indicates that the middleware is not compatible with
     *        the client.
     */
    incompatible_middleware_found
};

/**
 * @brief Thrown to indicate that the client was unable to connect to
 *        a running HAMcast middleware.
 * @ingroup ExceptionHandling
 */
class connection_to_middleware_failed : public std::exception
{

    const connection_error m_occurred_error;

 public:

    inline connection_to_middleware_failed(connection_error error_code) throw()
        : m_occurred_error(error_code) { }

    virtual ~connection_to_middleware_failed() throw();

    /**
     * @brief Get the general cause of this failure.
     * @returns the occurred error as C-style string
     */
    virtual const char* what() const throw();

    /**
     * @brief Get the occured error.
     * @returns the occurred error as {@link #connection_error}
     */
    inline connection_error occurred_error() throw()
    {
        return m_occurred_error;
    }

};

/**
 * @brief Thrown to indicate that the client lost the connection to the
 *        middleware.
 * @ingroup ExceptionHandling
 */
class connection_to_middleware_lost : public std::logic_error
{

 public:

    explicit connection_to_middleware_lost(const std::string& msg)
        : std::logic_error(msg) { }

};

/**
 * @brief Thrown to indicate that an IPC call failed because of an
 *        internal interface error.
 * @ingroup ExceptionHandling
 */
class internal_interface_error : public std::exception
{

    std::string m_what;

 public:

    explicit internal_interface_error(const std::string& err_msg);

    virtual const char* what() const throw();

    inline const std::string& what_str() const { return m_what; }

    virtual ~internal_interface_error() throw();

};

/**
 * @def HC_REQUIRE(req)
 * @ingroup ExceptionHandling
 * @brief Similiar to assert() but throws an exception if the requirement
 *        fails instead of calling terminate() and also logs the error with
 *        HC_LOG_FATAL().
 * @param req The requirement (source code).
 * @relates hamcast::requirement_failed
 * @throws hamcast::requirement_failed
 */

/**
 * @def HC_REQUIRE_VERBOSE(req,verbose_msg)
 * @ingroup ExceptionHandling
 * @brief Equal to #HC_REQUIRE(req) but uses @p verbose_msg as
 *        as error message instead of @p req.
 * @param req The requirement (source code).
 * @param verbose_msg An expression that could use shift operations
 *                    and results in an string.
 * @relates hamcast::requirement_failed
 * @throws hamcast::requirement_failed
 */

/**
 * @brief Thrown by #HC_REQUIRE(req)
 *        and #HC_REQUIRE_VERBOSE(req,verbose_msg)
 *        if the requirement <code>req</code> evaluates to false.
 * @ingroup ExceptionHandling
 */
class requirement_failed : public std::exception
{

    std::string m_what;

 public:

    requirement_failed(const std::string& msg);

    requirement_failed(int line, const char* file, const char* msg);

    requirement_failed(int line, const char* file, const std::string& msg);

    virtual ~requirement_failed() throw();

    virtual const char* what() const throw();

    /**
     * @brief Get the general cause of this exception.
     * @returns The failed requirement including source file name and line
     *         as string object.
     */
    inline const std::string& what_str() const { return m_what; }

};

} // namespace hamcast

#ifdef __GNUC__
inline void __attribute__((deprecated)) HAMCAST_REQUIRE_MACRO() { }
#elif defined(_MSC_VER)
inline __declspec(deprecated) void HAMCAST_REQUIRE_MACRO() { }
#else
inline void HAMCAST_REQUIRE_MACRO() { }
#endif

#define HC_REQUIRE(req)                                                        \
if (!(req))                                                                    \
{                                                                              \
    HC_LOG_FATAL("Requirement failed: " << BOOST_STRINGIZE(req));              \
    throw ::hamcast::                                                          \
    requirement_failed( __LINE__ , __FILE__ , BOOST_STRINGIZE(req) );          \
}

#define HAMCAST_REQUIRE(req)                                                   \
HAMCAST_REQUIRE_MACRO(); HC_REQUIRE(req)

#define HC_REQUIRE_VERBOSE(req, verbose_msg)                                   \
if (!(req))                                                                    \
{                                                                              \
    HC_LOG_FATAL("Requirement failed: " << BOOST_STRINGIZE(req));              \
    std::ostringstream err;                                                    \
    err << verbose_msg ;                                                       \
    throw ::hamcast::                                                          \
    requirement_failed( __LINE__ , __FILE__ , err.str() );                     \
}

#define HAMCAST_REQUIRE_VERBOSE(req)                                           \
HAMCAST_REQUIRE_MACRO(); HC_REQUIRE_VERBOSE(req)

#endif // HAMCAST_EXCEPTIONS_HPP
