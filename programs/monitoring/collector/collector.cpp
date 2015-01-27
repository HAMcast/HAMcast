#include <map>
#include <string>
#include <vector>

#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"
#include <boost/shared_ptr.hpp>

#include "collector.hpp"
#include "interface.hpp"


void  collector::deliver_node(boost::shared_ptr<node> p_node,std::string daemon_id){
    WriteLock w_lock(list_lock);
    node_list[daemon_id] = p_node;
    std::vector<interface> interfaces = p_node->get_interfaces();
    for(int i =0; i < interfaces.size();i++){
        interface inter = interfaces.at(i);
        daemon_map[inter.get_addr()] = daemon_id;
    }
}

void collector::remove_node(std::string daemon_id){
     WriteLock w_lock(list_lock);
//     boost::shared_ptr<node> p_node = node_list[daemon_id];
//     std::vector<interface> interfaces = p_node->get_interfaces();
//     for(int i =0; i < interfaces.size();i++){
//         interface inter = interfaces.at(i);
//         daemon_map.erase(inter.get_addr());
//     }
     node_list.erase(daemon_id);
}

std::string collector::to_string(){
    std::stringstream output;
    boost::property_tree::ptree pt;
    ReadLock r_lock(list_lock);
    int count = 0;
    std::map<std::string,boost::shared_ptr<node> >::const_iterator itr;
    for(itr = node_list.begin(); itr != node_list.end(); ++itr){
        std::string key = (*itr).first;
        boost::shared_ptr<node> n = (*itr).second;
        std::string s_count = boost::lexical_cast<std::string>(count);
        pt.add_child("nodes.node"+s_count, n->to_xml().second);
    }

    try{
        boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
        boost::property_tree::xml_parser::write_xml( output, pt,w );
    }
    catch (std::exception& e){
        std::cout << e.what() << std::endl;
    }

    return output.str();
}
