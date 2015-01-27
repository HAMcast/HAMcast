#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "interface.hpp"
#include "node.hpp"

using std::string;
using std::vector;

node::node(std::string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    m_name= pt.get("name","");
    m_img = pt.get("img",false);
    boost::property_tree::ptree pt2;
try{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("interfaces")){
        string key;
        pt2.clear();
        key = v.first.data();
        try{
            pt2 = pt.get_child("interfaces."+key);
//            std::stringstream output;
//            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
//            boost::property_tree::xml_parser::write_xml( output, pt2,w );
//            std::string input = output.str();
//            interface i(input);
            interface i(pt2);
            m_interface.push_back(i);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

node::node(string name,vector<interface> interfaceArg,bool img)
    :m_name(name),m_interface(interfaceArg), m_img(img)
{
}


std::pair<std::string,boost::property_tree::ptree> node::to_xml(){
    std::stringstream output;
    boost::property_tree::ptree pt;
    pt.add("name",m_name);
    pt.add("img",m_img);

    try{
    for(unsigned int i =0; i < m_interface.size();i++){
        pt.add_child("interfaces.interface"+boost::lexical_cast<string>(i), m_interface.at(i).to_xml().second);
    }
        boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
        boost::property_tree::xml_parser::write_xml( output, pt,w );
    }
    catch (std::exception& e){
        std::cout << e.what() << std::endl;
    }
    std::pair<std::string,boost::property_tree::ptree> res;
    res.first = output.str();
    res.second = pt;
    return res;
}
