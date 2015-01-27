#ifndef FUNCTION_WRAPPER_HPP
#define FUNCTION_WRAPPER_HPP

#include <string>
#include <vector>

#include "hamcast/hamcast.hpp"
#include "method_caller.hpp"

class function_wrapper : public method_caller
{
public:

    function_wrapper(std::string name);

private:

    std::vector<hamcast::uri> children_set(int& id, hamcast::uri& g);
    std::vector<hamcast::uri> parent_set(int& id, hamcast::uri& g);
    std::vector<hamcast::uri> neighbor_set(int& id);
    std::vector<std::pair<hamcast::uri,unsigned int> > group_set(int& id);
    bool is_img();
    std::vector<hamcast::interface_property> interfaces();
    std::string get_node(const std::vector<std::string>& args);
    std::string get_children_set(std::vector<std::string>& args);
    std::string get_parent_set(std::vector<std::string>& args);
    std::string get_neighbor_set(std::vector<std::string>& args);
    std::string get_group_set(std::vector<std::string>& args);
    std::string get_interfaces(std::vector<std::string>& args);
    std::string get_is_img(std::vector<std::string>& args);
};

#endif // FUNCTION_WRAPPER_HPP


