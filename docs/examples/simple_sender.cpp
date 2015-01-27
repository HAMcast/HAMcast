// this header contains all needed classes and
// functions to use the HAMcast library
#include "hamcast/hamcast.hpp"

// C++ STL include for std::cout
#include <iostream>

int main()
{
	// throws if no middleware was found
	hamcast::multicast_socket s;
	// a string message
	std::string hello_world = "Hello World!";
	// send hello_world via multicast socket s
	s.send("ip://239.0.1.1:1234", hello_world.size(), hello_world.c_str());
	// done
	std::cout << "DONE" << std::endl;
	return 0;
}

