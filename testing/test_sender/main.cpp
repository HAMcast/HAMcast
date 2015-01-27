#include <iostream>
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_logging.h"

#include <boost/cstdint.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

int main(int argc, char** argv)
{
	hamcast::uri group;
	if (argc == 2)
	{
		std::string uri_str = argv[1];
		group = uri_str;
	}
	else
	{
		group = std::string("ip://239.0.1.1:1234");
	}

	std::cout << "group = " << group.str() << std::endl;

	hamcast::multicast_socket s;

	boost::system_time tout = boost::get_system_time();
	boost::uint32_t num = 0;
	std::cout << std::endl;
	for (;;)
	{
		std::string msg = "Hello World Nr. ";
		msg += boost::lexical_cast<std::string>(++num);
		s.send(group, msg.size(), msg.c_str());
		std::cout << "\r" << msg;
		std::cout.flush();
		tout = boost::get_system_time();
		tout += boost::posix_time::milliseconds(1);
//		boost::this_thread::sleep(tout);
	}
	std::cout << "DONE" << std::endl;
	return 0;
}
