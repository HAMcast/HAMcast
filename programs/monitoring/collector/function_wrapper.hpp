 #ifndef MY_METHOD_CALLER_HPP
#define MY_METHOD_CALLER_HPP
#include "method_caller.hpp"
#include <string>
#include <collector.hpp>
#include "node.hpp"
#include "interface.hpp"
#include "group.hpp"

class function_wrapper : public method_caller
{
private:
    collector& m_collector;
    int lcp(std::string& n1,std::string& n2);
    std::string group_list(std::vector<std::string> args);
    std::string node_list(std::vector<std::string> args);
    std::string group_data(std::vector<std::string> args);
    std::string node_data(std::vector<std::string> args);
    std::string group_tree(std::vector<std::string> args);
public:
    /**
      *@brief creates the function wrapper of the collector
      */
    function_wrapper(collector& collector);
};

#endif // MY_METHOD_CALLER_HPP
