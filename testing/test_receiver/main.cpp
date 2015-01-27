#include <iostream>
#include <algorithm>

#include "hamcast/ipc.hpp"
#include "hamcast/hamcast.hpp"

#include <boost/thread.hpp>

int main(int argc, char** argv)
{
    hc_set_default_log_fun(HC_LOG_TRACE_LVL);

    hamcast::uri group;
    if (argc == 2)
    {
        std::string uri_str = argv[1];
        group = uri_str;
    }
    else
    {
        std::string uri = "ip://239.0.1.1:1234";
        group = uri;
    }

    std::cout << "group = " << group.str() << std::endl;

    hamcast::multicast_socket s;
    s.join(group);

    boost::system_time tout = boost::get_system_time();
    tout += boost::posix_time::seconds(1);
    int packages = 0;
    std::cout << "enter loop ... " << std::endl;
    for (;;)
    {
        ++packages;
        hamcast::multicast_packet mp = s.receive();
        const char* msg = reinterpret_cast<const char*>(mp.data());
        std::cout << "\rmsg (size: " << mp.size() << "): ";
        std::copy(msg, msg + mp.size(), std::ostream_iterator<char>(std::cout));
        std::cout.flush();
    }
    return 0;
}
