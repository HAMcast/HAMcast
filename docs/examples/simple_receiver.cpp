// this header contains all needed classes and functions
// to use the HAMcast library
#include "hamcast/hamcast.hpp"

// C++ STL includes for std::cout and std::copy()
#include <iostream>
#include <algorithm>

int main()
{
	// throws if no middleware was found
	hamcast::multicast_socket s;
	// join a multicast group
	s.join("ip://239.0.1.1:1234");
	// receive one packet
	hamcast::multicast_packet mp = s.receive();
	// interpret its content as C-string
	const char* msg = reinterpret_cast<const char*>(mp.data());
	// print the received text
	std::copy(msg, msg + mp.size(), std::ostream_iterator<char>(std::cout));
	std::cout << std::endl;
	// done
	return 0;
}

