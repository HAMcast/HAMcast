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

#include <cstdlib>      // abort()
#include <fstream>      // fstream
#include <getopt.h>     // option/args parsing
#include <iostream>     // cout, cerr, endl
#include <string>       // string
#include <typeinfo>     // typeid(), type_info
#include <exception>    // set_terminate()

// mkdir
#include <signal.h>     // sigaction
#include <unistd.h>     // unlink
#include <sys/stat.h>   // mkdir, rmdir
#include <fcntl.h>      // creat()

#include <boost/regex.hpp>
#include <boost/thread.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/interprocess/exceptions.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "session.hpp"
#include "middleware.hpp"
#include "hamcast/hamcast_logging.h"

#include "hamcast/util/sink.hpp"
#include "hamcast/util/source.hpp"
#include "hamcast/util/socket_io.hpp"
#include "hamcast/util/config_map.hpp"
#include "hamcast/util/buffered_sink.hpp"
#include "hamcast/util/buffered_source.hpp"
#include "hamcast/ipc/middleware_configuration.hpp"

#ifdef WIN_32
#else
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cxxabi.h>         // abi::__cxa_demangle
#endif

#ifdef HC_USE_UNIX_SOCKETS
#include <cstdio>
#include <cstdlib>
#include <sys/un.h>
#include <sys/socket.h>
#endif

using std::cout;
using std::cerr;
using std::endl;

using namespace hamcast;
using namespace hamcast::ipc;
using namespace hamcast::util;
using namespace hamcast::middleware;

typedef std::pair<source::ptr, sink::ptr> io_ptrs;

namespace {

// default values, can be changed by runtime args
const std::string c_default_ini_file ("middleware.ini");

// to parse long opts
option long_options[] = {
//    {"daemonize",       no_argument,        &daemonize, 1},
    {"help",        no_argument,        0,  'h'},
    {"ini-file",    required_argument,  0,  'i'},
    {0, 0, 0, 0}
};

typedef boost::uint16_t ui16;

template<typename T>
bool in_range(T x, T min_value, T max_value)
{
    return x >= min_value && x <= max_value;
}

bool verify_client(const source::ptr& in)
{
    HC_LOG_TRACE("");
    deserializer d(in);
    boost::uint32_t value;
    // the first value must be the magic number
    d >> value;
    if (value != magic_number)
    {
        HC_LOG_DEBUG("Wrong magic number");
        return false;
    }
    // the second value must be the major version of the client
    d >> value;
    if (!in_range(value, min_compatile_major_version, major_version))
    {
        HC_LOG_DEBUG("Incompatible major version");
        return false;
    }
    // the third value must be the minor version of the client
    d >> value;
    if (!in_range(value, min_compatile_minor_version, minor_version))
    {
        HC_LOG_DEBUG("Incompatible minor version");
        return false;
    }
    HC_LOG_DEBUG("New client accepted");
    return true;
}

bool client_compatible(const io_ptrs& io, boost::uint32_t max_msg_size)
{
    HC_LOG_TRACE("");
    boost::uint8_t result = verify_client(io.first) ? 1 : 0;
    // lifetime scope of s
    {
        serializer s(io.second);
        s << result;
        s << max_msg_size;
    }
    return result == 1;
}

void wait4clients_loop(boost::uint32_t max_msg_size)
{
    HC_LOG_TRACE("");

    cout << "run loop ..." << endl;

    // start server at random port
    hamcast::util::native_socket ssock; // server socket
    hamcast::util::native_socket csock; // client socket
    int cl_len = sizeof(sockaddr_in);
#   ifndef HC_USE_UNIX_SOCKETS
    sockaddr_in cl_addr; // client address
    sockaddr_in sv_addr; // server address
    memset(&sv_addr, 0, sizeof(sockaddr_in));
    memset(&cl_addr, 0, sizeof(sockaddr_in));
    HC_REQUIRE((ssock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) != 0);
    sv_addr.sin_family = AF_INET;
    sv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bool bind_succeeded = false;
    while (!bind_succeeded)
    {
        // choose a random port > 1024
        ui16 port = static_cast<ui16>(((rand() % ((0xFFFF) - 1025)) + 1025));
        sv_addr.sin_port = htons(port);
        // try to bind to the local address
        if (bind(ssock, (sockaddr*) &sv_addr, sizeof(sockaddr_in)) == 0)
        {
            HC_LOG_DEBUG("Middleware runs on port " << port);
            // lifetime scope of mc
            {
                ipc::middleware_configuration mc(port);
                mc.write();
            }
            bind_succeeded = true;
        }
    }
#   else
    sockaddr_un cl_addr; // client address
    sockaddr_un sv_addr; // server address
    memset(&sv_addr, 0, sizeof(sockaddr_un));
    memset(&cl_addr, 0, sizeof(sockaddr_un));
    HC_REQUIRE((ssock = socket(AF_UNIX, SOCK_STREAM, 0)) != 0);
    sv_addr.sun_family = AF_UNIX;
    unlink(socket_path);
    strncpy(sv_addr.sun_path, socket_path, sizeof(sv_addr.sun_path) - 1);
    if (bind(ssock, (sockaddr*) &sv_addr, sizeof(sv_addr)) == -1) {
        perror("bind error");
        throw std::runtime_error("cannot bind to unix domain socket");
    }
#   endif
    // max 10 pending connections
    HC_REQUIRE(listen(ssock, 10) == 0);
    for (;;)
    {
        HC_REQUIRE((csock = accept(ssock,
                                   (sockaddr*) &cl_addr,
                                   (socklen_t*) &cl_len)) > 0);
        socket_io::ptr sio(socket_io::create(csock));
        source::ptr in(new buffered_source<>(sio.get()));
        sink::ptr out(new buffered_sink<>(sio.get()));
        if (client_compatible(io_ptrs(in, out), max_msg_size))
        {
            hamcast::middleware::session::create(in, out);
        }
        else
        {
            HC_LOG_DEBUG("Client rejected");
        }
    }
}

void set_log_level(int lvl)
{
    if (hc_logging_enabled())
    {
        hc_set_default_log_fun(lvl);
    }
    else
    {
        std::cout << "WARNING: log level ignored"
                     " (libHAMcast compiled without logging)"
                  << endl;
    }
}

boost::interprocess::file_lock* middleware_lock;//(lock_file.c_str());

int run_middleware(const std::string& ini_file)
{
    //boost::interprocess::file_lock middleware_lock(lock_file.c_str());
    //if (!middleware_lock.try_lock())
    //{
    //    cerr << "Only one instance of the middleware could be active." << endl
    //         << "If you are sure that there is no other instance is running "
    //         << "then delete the file \""
    //         << lock_file << "\"." << endl;
    //    return 2;
    //}
    // lifetime scope of ini
    {
        config_map ini;
        ini.read_ini(ini_file);
        const string_map& global = ini["global"];
        for (string_map::const_iterator i = global.begin();
             i != global.end();
             ++i)
        {
            if (i->first == "log_level")
            {
                const std::string& lvl = i->second;
                if (lvl == "trace")
                {
                    set_log_level(HC_LOG_TRACE_LVL);
                }
                else if (lvl == "debug")
                {
                    set_log_level(HC_LOG_DEBUG_LVL);
                }
                else if (lvl == "info")
                {
                    set_log_level(HC_LOG_INFO_LVL);
                }
                else if (lvl == "warn")
                {
                    set_log_level(HC_LOG_WARN_LVL);
                }
                else if (lvl == "error")
                {
                    set_log_level(HC_LOG_ERROR_LVL);
                }
                else if (lvl == "fatal")
                {
                    set_log_level(HC_LOG_FATAL_LVL);
                }
                else if (lvl == "none")
                {
                    hc_set_log_fun(0);
                }
                else
                {
                    cerr << "illegal log level; expected one of "
                            "trace, debug, info, warn, error, fatal or none"
                         << endl;
                }
            }
            else
            {
                cerr << "unknown key found: \"" << i->first << "\" "
                        "(valid keys: log_level)"
                     << endl;
            }
        }
        // check for [global] and max_msg_size
        if (!ini.has_group("global"))
        {
            cerr << "no [global] section found in ini file" << endl;
        }
        boost::uint32_t max_msg_size = default_max_msg_size;
        const std::string& mmsize = ini.get("global", "max_msg_size");
        if (mmsize.empty())
        {
            cerr << "no max_msg_size specified in [global] (use default)"
                 << endl;
        }
        else
        {
            try
            {
                max_msg_size = boost::lexical_cast<boost::uint32_t>(mmsize);
            }
            catch (...) { } // use default on error
        }
        // check if each module has a file assigned
        for (config_map::const_iterator i = ini.begin(); i != ini.end(); ++i)
        {
            const std::string& section = i->first;
            if (section != "global")
            {
                const std::string& fname = ini.get(i, "file");
                if (fname.empty())
                {
                    cerr << "no file name specified for module "
                         << section << endl;
                }
                else
                {
                    tech_module::load(fname.c_str(), i->second, max_msg_size);
                }
            }
        }
        wait4clients_loop(max_msg_size);
        return 0;
    }
}

volatile int exit_value = EXIT_SUCCESS;

void erase_trailing_slash(std::string& path)
{
    while (path.size() > 1 && *path.rbegin() == '/')
    {
        path.resize(path.size() - 1);
    }
}

void ensure_trailing_slash(std::string& path)
{
    if (path.empty())
    {
        throw std::runtime_error("path.emtpy()");
    }
    if (path.size() > 1)
    {
        if (*path.rbegin() != '/')
        {
            path += '/';
        }
    }
}

void remove_file(const std::string& cfile)
{
    if (boost::filesystem::exists(cfile.c_str()))
    {
        if (unlink(cfile.c_str()) != 0)
        {
            cerr << "could not remove "
                 << meeting_point << config_filename
                 << endl;
        }
    }
}

void cleanup()
{
    // TODO: check if this signal handler is safe
    if (middleware_lock)
    {
        delete middleware_lock;
    }
    std::string path = meeting_point;
    std::string root = meeting_point_root;
    ensure_trailing_slash(path);
    // remove middleware config file
    std::string cfile = path;
    cfile += config_filename;
    remove_file(cfile);
    // remove lock file
    cfile = path;
    cfile += lock_filename;
    remove_file(cfile);
    erase_trailing_slash(path);
    while (path.size() > root.size())
    {
        if (rmdir(path.c_str()) == 0)
        {
            cout << "removed directory " << path << endl;
        }
        else
        {
            cout << "could not remove directory " << path << endl;
            return;
        }
        std::string::size_type new_size = path.find_last_of('/');
        path.resize(new_size);
    }
   tech_module::unload_all();
}

std::string demangle(const std::type_info& tinfo)
{
    size_t size;
    int status;
    char* undecorated = abi::__cxa_demangle(tinfo.name(), 0, &size, &status);
    if (status != 0)
    {
        return tinfo.name();
    }
    else
    {
        std::string result(undecorated, size);
        free(undecorated);
        return result;
    }
}

void print_exception(const std::exception& e)
{
    cerr << "terminate() called after throwing an instance of "
         << demangle(typeid(e))
         << "\nwhat(): " << e.what()
         << endl;
}

void terminate_handler()
{
    // throw exception again
    try { throw; }
    // and get the reason
    catch (std::exception& e)
    {
        print_exception(e);
    }
    catch (...)
    {
        cerr << "terminate() called after throwing an unknown exception"
             << endl;
    }
    cleanup();
    exit(3);
}

enum creation_result
{
    cr_ok,
    cr_exist,
    cr_failure
};

creation_result create_directory(const std::string& dir)
{
    if (mkdir(dir.c_str(), 0755) == 0)
    {
        return cr_ok;
    }
    if (errno == EEXIST)
    {
        return cr_exist;
    }
    else
    {
        return cr_failure;
    }
}

void create_file(const std::string& filename, bool throw_if_file_exists = false)
{
    int fhandle = creat(filename.c_str(), 0644);
    if (fhandle != -1)
    {
        close(fhandle);
    }
    else if (throw_if_file_exists || errno != EEXIST)
    {
        std::ostringstream errstr;
        errstr << "could not create " << filename;
        if (errno == EEXIST)
        {
            errstr << " (file already exists)";
        }
        else
        {
            errstr << " (error code: " << errno << ")";
        }
        throw std::runtime_error(errstr.str());
    }
}


void initialize()
{
    umask(0);
    std::string path = meeting_point;
    std::string root = meeting_point_root;
    if (path.empty())
    {
        throw std::logic_error("meeting_point is an empty string");
    }
    if (path.at(0) != '/')
    {
        throw std::logic_error("meeting_point is not an absolute path");
    }
    ensure_trailing_slash(path);
    std::string::size_type pos = path.find('/', root.size());
    for ( ; pos < path.size(); pos = path.find('/', pos + 1))
    {
        std::string sub = path.substr(0, pos);
        switch (create_directory(sub))
        {
         case cr_ok:
            cout << "created directory " << sub << endl;
            break;
         case cr_exist:
            cout << "direcory " << sub << " already exists" << endl;
            break;
         case cr_failure:
            cerr << "failure during creation of directory " << sub << endl;
            abort();
        }
    }
    // create config file
    std::string fname = path;
    fname += config_filename;
    create_file(fname);
    // create lock file
    fname = path;
    fname += lock_filename;
    create_file(fname, true);
    // get file lock
    middleware_lock = new boost::interprocess::file_lock(fname.c_str());
    if (!middleware_lock->try_lock())
    {
        throw std::runtime_error("Could not get interprocess file lock");
    }
}

void sigterm_handler(int)
{
    cout << endl;
    cleanup();
    exit(4);
}

void set_sigterm_handler()
{
    signal(SIGTERM, sigterm_handler);
    signal(SIGABRT, sigterm_handler);
    signal(SIGINT, sigterm_handler);
}

void usage (const std::string& program)
{
        cout << endl;
        cout << "USAGE" << endl;
        cout << "\t" << program << " [-h] [-i <INI>]" << endl;
        cout << endl;
}
void help (const std::string& program)
{
    usage(program);
    cout << "OPTION" << endl;
    cout << "\t-h, --help" << endl;
    cout << "\t\tPrint this help screen." << endl;
    cout << "\t-i, --ini-file INI" << endl;
    cout << "\t\tSet name and path of INI file with middleware configuration (middleware.ini)." << endl;
    cout << endl;
    cout << "NOTES" << endl;
    cout << "\tIf INI exists in run path of middleware, no arguments are needed." << endl;
    cout << endl;
    cout << "AUTHORS" << endl;
    cout << "\tDominik Charousset <dominik.charousset (at) haw-hamburg.de>" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de>" << endl;
    cout << endl;
}

} // namespace <anonymous>

int main(int argc, char** argv)
{
    std::string ini_file = c_default_ini_file;
    // option parsing
    std::string program (argv[0]);
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hi:",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0: {
                if (long_options[option_index].flag != 0)
                    break;
                cout << "option " <<
                    long_options[option_index].name;
                if (optarg)
                    cout << " with arg " << optarg;
                cout << endl;
                break;
            }
            case 'i': {
                ini_file = std::string (optarg);
                if (ini_file.size() < 5) {
                    cerr << "Invalid INI file name! Should be '<name>.ini'." << endl;
                    exit (EXIT_FAILURE);
                }
                break;
            }
            case 'h': {
                help (program);
                exit (EXIT_SUCCESS);
                break;
            }
            default: {
                usage (program);
                exit (EXIT_FAILURE);
                break;
            }
        }
    }
    // end option parsing

    srand(time(NULL));
    initialize();

#   ifndef HC_USE_UNIX_SOCKETS
    cout << "note: this middleware uses IP localhost sockets for IPC" << endl;
#   else
    cout << "note: this middleware uses UNIX domain sockets for IPC" << endl;
#   endif

    // init stuff
    std::set_terminate(terminate_handler);
    set_sigterm_handler();
    int exit_reason = 0;
    try
    {
        run_middleware(ini_file);
    }
    catch (boost::interprocess::interprocess_exception& e)
    {
        exit_reason = 1;
        cerr << "Only one instance of the middleware could be active." << endl
             << "If you are sure that there is no other instance is running "
             << "then delete the file \""
             << meeting_point << lock_filename << "\"." << endl;
        std::string err_msg = "unable to get interprocess lock: ";
        err_msg += e.what();
        print_exception(std::runtime_error(err_msg));
    }
    catch (std::exception& e)
    {
        exit_reason = 2;
        print_exception(e);
    }
    catch (...)
    {
        exit_reason = 5;
    }
    cleanup();
    return exit_reason;
}
