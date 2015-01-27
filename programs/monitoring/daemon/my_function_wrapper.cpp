#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"
#include "http_message.hpp"
#include "interface.hpp"
#include "node.hpp"
//#include "my_function_wrapper.hpp"

my_function_wrapper::my_function_wrapper(std::string name)
    : method_caller(name)
{
    register_function("/get_node", (function_call)&my_function_wrapper::get_node);
    register_function("/children_set", (function_call)&my_function_wrapper::get_children_set);
    register_function("/parent_set", (function_call)&my_function_wrapper::get_parent_set);
    register_function("/neighbor_set", (function_call)&my_function_wrapper::get_neighbor_set);
    register_function("/group_set", (function_call)&my_function_wrapper::get_group_set);
    register_function("/interface", (function_call)&my_function_wrapper::get_interfaces);
    register_function("/is_img", (function_call)&my_function_wrapper::get_is_img);


}


std::vector<hamcast::uri> my_function_wrapper::children_set(int& id, hamcast::uri& g){
    std::vector<hamcast::uri> ret;
    try{
        ret = hamcast::ipc::children_set(id,g);
    }
    catch(std::exception& e){
        std::cerr << "calling children_set failed " << e.what() << std::endl;
    }
    return ret;
}

std::vector<hamcast::uri> my_function_wrapper::parent_set(int& id, hamcast::uri& g){
    std::vector<hamcast::uri> ret;
    try{
        ret = hamcast::ipc::parent_set(id,g);
    }
    catch(std::exception& e){
        std::cerr << "calling parent_set failed " << e.what() << std::endl;
    }
    return ret;
}

std::vector<hamcast::uri> my_function_wrapper::neighbor_set(int& id){
    std::vector<hamcast::uri> ret;
    try{
        ret = hamcast::ipc::neighbor_set(id);
    }
    catch(std::exception& e){
        std::cerr << "calling neighbor_set failed " << e.what() << std::endl;
    }
    return ret;
}

std::vector<std::pair<hamcast::uri,unsigned int> > my_function_wrapper::group_set(int& id){
    std::vector<std::pair<hamcast::uri,unsigned int> > ret;
    try{
        ret = hamcast::ipc::group_set(id);
    }
    catch(std::exception& e){
        std::cerr << "calling group_set failed " << e.what() << std::endl;
    }
    return ret;
}

bool my_function_wrapper::is_img(){
    bool ret = false;
    try{
        ret = hamcast::ipc::is_img();
    }
    catch(std::exception& e){
        std::cerr << "calling is_img failed" << e.what() << std::endl;
    }
    return ret;
}

std::vector<hamcast::interface_property> my_function_wrapper::interfaces(){
        std::vector<hamcast::interface_property> ret;
    try{
        ret = hamcast::ipc::get_interfaces();
    }
    catch(std::exception& e){
        std::cerr << "calling is_img failed" << e.what() << std::endl;
    }
    return ret;
}

std::string my_function_wrapper::get_node(const std::vector<std::string>& args)
{
    std::vector<hamcast::interface_property> p_interface_probs = interfaces();
    bool img = is_img();
    std::vector<interface> interface_list;
    for(int i = 0; i < p_interface_probs.size();i++){
        hamcast::interface_property prob = p_interface_probs.at(i);
        int id = prob.id;
        std::string name = prob.name;
        std::string tech = prob.technology;
        hamcast::uri addr(prob.address);
        std::vector<group>     group_list;
        std::vector<hamcast::uri> p_neighbors = neighbor_set(id);
        std::vector<std::pair<hamcast::uri,unsigned int> > groups = group_set(id);
        for(int g =0; g < groups.size();g++){
            hamcast::uri group_uri(groups.at(g).first);
            std::vector<hamcast::uri> p_children = children_set(id,group_uri);
            std::vector<hamcast::uri> p_parent = parent_set(id,group_uri);
            if(p_children.empty()){
                p_children.push_back("ip://0.0.0.0");
            }
            if(p_parent.empty()){
                p_parent.push_back("ip://0.0.0.0");
            }
            group p_group(group_uri,p_parent.front(),p_children);
            group_list.push_back(p_group);
        }
        interface p_interface(name,tech,id,addr,group_list,p_neighbors);
        interface_list.push_back(p_interface);
    }
    node n(get_daemon_id(),interface_list,img);
    return n.to_xml().first;
}


std::string my_function_wrapper::get_children_set(std::vector<std::string>& args){
    if(args.size() < 2){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        std::stringstream output;
        try{
            int id = boost::lexical_cast<int>(args.at(0));
            hamcast::uri u(args.at(1));
            std::vector<hamcast::uri> children = children_set(id,u);
            boost::property_tree::ptree pt;
            for(unsigned int i =0; i < children.size();i++){
                std::string arg = children.at(i).str();
                pt.add("children.child",arg);
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
        return output.str();
    }
}

std::string my_function_wrapper::get_parent_set(std::vector<std::string>& args){
    if(args.size() < 2){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        try{
            int id = boost::lexical_cast<int>(args.at(0));
            hamcast::uri u(args.at(1));
            std::vector<hamcast::uri> parents = parent_set(id,u);
            std::stringstream output;
            boost::property_tree::ptree pt;
            for(unsigned int i =0; i < parents.size();i++){
                std::string arg = parents.at(i).str();
                pt.add("parents.parent",arg);
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
            return output.str();
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string my_function_wrapper::get_neighbor_set(std::vector<std::string>& args){
    if(args.size() < 1){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        try{
            int id = boost::lexical_cast<int>(args.at(0));
            std::vector<hamcast::uri> neighbors = neighbor_set(id);
            std::stringstream output;
            boost::property_tree::ptree pt;
            for(unsigned int i =0; i < neighbors.size();i++){
                std::string arg = neighbors.at(i).str();
                pt.add("neighbors.neighbor",arg);
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
            return output.str();
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string my_function_wrapper::get_group_set(std::vector<std::string>& args){
    if(args.size() < 1){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        try{
            int id = boost::lexical_cast<int>(args.at(0));
            std::vector<std::pair<hamcast::uri,unsigned int> > groups = group_set(id);
            std::stringstream output;
            boost::property_tree::ptree pt;
            for(unsigned int i =0; i < groups.size();i++){
                std::string arg = groups.at(i).first.str();
                pt.add("groups.group",arg);
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
            return output.str();
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string my_function_wrapper::get_interfaces(std::vector<std::string>& args){
    if(args.size() !=0){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        try{
            std::vector<hamcast::interface_property> interface = interfaces();
            std::stringstream output;
            boost::property_tree::ptree pt;
            for(int i =0; i < interface.size();i++){
                hamcast::interface_property ip = interface.at(i);
                pt.add("interfaces.interface.name",ip.name);
                pt.add("interfaces.interface.tech",ip.technology);
                pt.add("interfaces.interface.id",ip.id);
                pt.add("interfaces.interface.addr",ip.address);
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
            return output.str();
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
    }
}

std::string my_function_wrapper::get_is_img(std::vector<std::string>& args){
    if(args.size() !=0){
        std::string errcode = "404";
        std::string payload;
        http_message errmsg(errcode,payload);
        return errmsg.to_string();
    }
    else{
        try{
            std::stringstream output;
            boost::property_tree::ptree pt;
            bool img = is_img();
            if (img) {
                pt.add("is_img","true");
            }
            else {
                pt.add("is_img","false");
            }
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
            return output.str();
        }
        catch (std::exception& e){
            std::cerr << e.what() << std::endl;
        }
    }
}
