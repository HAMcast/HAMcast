#include "hamcast_interface.hpp"
#include <string>
#include <vector>
#include "hamcast_group.hpp"
#include <hamcast/hamcast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <sstream>

using namespace std;
using hamcast::uri;
using namespace boost::property_tree;

hamcast_interface::hamcast_interface(string& xml)
{
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    m_name = QString::fromStdString(pt.get("name",""));
    m_tech= QString::fromStdString(pt.get("tech",""));
    m_id= pt.get("id",0);
    m_addr = hamcast::uri(pt.get("addr",""));

    boost::property_tree::ptree pt2;
    try{
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("groups")){
            string key;
            pt2.clear();
            key = v.first.data();

            pt2 = pt.get_child("groups."+key);
            std::stringstream output;
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt2,w );
            std::string input = output.str();
            hamcast_group g(input);
            m_groups.insert(g.get_name(),g);

        }
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
}

hamcast_interface::hamcast_interface(boost::property_tree::ptree pt){
    m_name = QString::fromStdString(pt.get("name",""));
    m_tech= QString::fromStdString(pt.get("tech",""));
    m_id= pt.get("id",0);
    m_addr = hamcast::uri(pt.get("addr",""));

    boost::property_tree::ptree pt2;
    try{
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("groups")){
            string key;
            pt2.clear();
            key = v.first.data();
            pt2 = pt.get_child("groups."+key);
            hamcast_group g(pt2);
            m_groups.insert(g.get_name(),g);

        }
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
}


hamcast_interface::hamcast_interface(QString addr,int id,QString name,QString tech)
    : m_addr(addr.toStdString()),m_id(id),m_name(name),m_tech(tech)
{
}


bool hamcast_interface:: operator==(const hamcast_interface &other) const {
    int comp_1;
    int comp_2;
    int comp_3;
    comp_1 = m_name.compare(other.get_name());
    comp_2 = m_tech.compare(other.get_tech());
    comp_3 = m_addr.compare(other.get_addr());
    if(comp_1 != 0 || comp_2 != 0 ||comp_3 != 0) return false;
    if(m_id != other.get_id() || m_neighbors != other.get_neighbors() || m_groups != other.get_groups()) return false;
    return true;
}

