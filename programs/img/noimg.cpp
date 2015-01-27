#include <string>
/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

using namespace hamcast;

int main (int argc, char** argv)
{
    if ((argc >  1) && (std::string(argv[1]) == "true")) {
        ipc::is_img(true);
        std::cout << "Enable IMG flag ..." << std::endl;
    } else {
        ipc::is_img(false);
        std::cout << "Disable IMG flag ..." << std::endl;
    }
}
