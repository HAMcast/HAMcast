#include "hamcast_group.hpp"
#include <string>
#include <hamcast/hamcast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>
#include <boost/foreach.hpp>

using hamcast::uri;
using std::string;


hamcast_group::hamcast_group(){

}


hamcast_group::hamcast_group(std::string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    hamcast::uri name(pt.get("name",""));
    hamcast::uri parent(pt.get("parent",""));
    try{
        QList<hamcast::uri> children;
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("children")){
            children.append(hamcast::uri(v.second.data()));
        }
        m_name =name;
        m_parent = parent;
        m_children = children;
    }
    catch (std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

hamcast_group::hamcast_group(boost::property_tree::ptree pt){
    hamcast::uri name(pt.get("name",""));
    hamcast::uri parent(pt.get("parent",""));
    try{
        QList<hamcast::uri> children;
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("children")){
            children.append(hamcast::uri(v.second.data()));
        }
        m_name =name;
        m_parent = parent;
        m_children = children;
    }
    catch (std::exception &e){
        std::cout << e.what() << std::endl;
    }
}

hamcast_group::hamcast_group(uri& name, uri& parent, QList<uri> children)
    : m_name(name),m_parent(parent),m_children(children)
{
}

bool hamcast_group:: operator==(const hamcast_group &other) const {
    int comp;
    comp = m_parent.compare(other.get_parent());
    if(comp !=0) return false;
    comp = m_name.compare(other.get_name());
    if(comp !=0) return false;
    if(m_children == other.get_children()) return true;
}
