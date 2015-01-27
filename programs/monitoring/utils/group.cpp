#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "hamcast/hamcast.hpp"
#include "group.hpp"

using std::string;
using std::vector;
using hamcast::uri;

group::group(string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    uri name(pt.get("name",""));
    uri parent(pt.get("parent",""));

    vector<uri> children;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("children")){
        children.push_back(uri(v.second.data()));
    }
    m_name =name;
    m_parent = parent;
    m_children = children;
}

group::group(boost::property_tree::ptree pt){
    uri name(pt.get("name",""));
    uri parent(pt.get("parent",""));

    vector<uri> children;
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("children")){
        children.push_back(uri(v.second.data()));
    }
    m_name =name;
    m_parent = parent;
    m_children = children;
}

std::pair<string,boost::property_tree::ptree> group::to_xml(){
    std::stringstream output;
    boost::property_tree::ptree pt;
    try{
        pt.add("name",m_name.str());
        pt.add("parent",m_parent.str());
        for(unsigned int i =0; i < m_children.size();i++){
            string arg = m_children.at(i).str();
            pt.add("children.child",arg);
        }

        boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
        boost::property_tree::xml_parser::write_xml( output, pt,w );
    }
    catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    std::pair<string,boost::property_tree::ptree> res;
    res.first = output.str();
    res.second = pt;
    return res;
}
