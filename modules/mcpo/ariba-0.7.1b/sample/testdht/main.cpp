#include <string>
#include "ariba/utility/system/StartupWrapper.h"
#include "DHTTest.h"

using std::string;
using ariba::utility::StartupWrapper;
using ariba::application::dhttest::DHTTest;

int main( int argc, char** argv ) {

	// get config file
	string config = "./settings.cnf";
	if (argc >= 2) config = argv[1];

	StartupWrapper::initConfig( config );
	StartupWrapper::startSystem();

	// this will do the main functionality and block
	DHTTest test;
	StartupWrapper::startup(&test);

	// --> we will run blocking until <enter> is hit

	StartupWrapper::shutdown(&test);
	StartupWrapper::stopSystem();

	return 0;
}
