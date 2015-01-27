#include "include/Receiver.h"
#include "ariba/utility/system/StartupWrapper.h"
#include "ariba/utility/configuration/Configuration.h"
#include <string>;
using ariba::utility::StartupWrapper;

int main(int argc, char *argv[])
{


    std::cout << "running..." << std::endl;
  //   StartupWrapper::initConfig( config );
    StartupWrapper::startSystem();
    std::cout << "argc: "<< argc << std::endl;
    if(argc>1)StartupWrapper::initConfig(argv[1]);

           //  AribaModule* ariba=new AribaModule();

    string nodename("mcponode-default");
    string endpoint("tcp{5004};");
    string bootstrap("hamcast{ip{127.0.0.1};tcp{5005}};");
    string spovnetname("hamcast");
    uint32_t port=666;
    if(argc>1 && Configuration::haveConfig()){
        Configuration& config = Configuration::instance();

        nodename=config.read<string>("nodename");
        endpoint=config.read<string>("endpoint");
        bootstrap=config.read<string>("bootstrap");
        spovnetname=config.read<string>("spovnetname");
        port=config.read<int>("port");


    }
    ariba::application::Receiver receiver(nodename, endpoint, bootstrap, spovnetname, port);

           StartupWrapper::startup(&receiver);


            StartupWrapper::stopSystem();

    return 0;
}
