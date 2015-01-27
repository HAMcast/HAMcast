#include <string>
#include "ariba/utility/system/StartupWrapper.h"
#include "PingPong.h"

using std::string;
using ariba::utility::StartupWrapper;
using ariba::application::pingpong::PingPong;

int main( int argc, char** argv ) {

	// get config file
	string config = "../../etc/pingpong/settings.cnf";
	if (argc >= 2) config = argv[1];

	StartupWrapper::initConfig( config );
	StartupWrapper::startSystem();

	// this will do the main functionality and block
	PingPong mcpotester;
	StartupWrapper::startup(&mcpotester);

	// this will run blocking until <enter> is hit
	StartupWrapper::shutdown(&mcpotester);
	StartupWrapper::stopSystem();

	return 0;
}
