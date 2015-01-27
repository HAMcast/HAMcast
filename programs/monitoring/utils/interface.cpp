#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include "hamcast/hamcast.hpp"
#include "group.hpp"
#include "interface.hpp"

using std::string;
using std::vector;

using hamcast::uri;

interface::interface()
{
}

interface::interface(string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    m_name= pt.get("name","");
    m_tech= pt.get("tech","");
    m_id= pt.get("id",0);
    m_addr = hamcast::uri(pt.get("addr",""));

    boost::property_tree::ptree pt2;

    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("groups")){
        string key;
        pt2.clear();
        key = v.first.data();
        try{
            pt2 = pt.get_child("groups."+key);
            std::stringstream output;
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt2,w );
            std::string input = output.str();
            group g(input);

            m_groups.push_back(g);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
}


interface::interface(boost::property_tree::ptree pt){
    m_name= pt.get("name","");
    m_tech= pt.get("tech","");
    m_id= pt.get("id",0);
    m_addr = hamcast::uri(pt.get("addr",""));

    init(pt);
}

void interface::init(boost::property_tree::ptree pt){

    boost::property_tree::ptree pt2;
    boost::optional< boost::property_tree::ptree& > child = pt.get_child_optional( "groups" );
    if( !child )
    {
        // child node is missing
    }
    else{
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("groups")){
            string key;
            pt2.clear();
            key = v.first.data();
            try{
                pt2 = pt.get_child("groups."+key);
                //            std::stringstream output;
                //            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
                //            boost::property_tree::xml_parser::write_xml( output, pt2,w );
                //            std::string input = output.str();
                //            group g(input);
                group g(pt2);
                m_groups.push_back(g);
            }
            catch(std::exception& e){
                std::cout << e.what() << std::endl;
            }
        }
    }
}

void interface::init(std::string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    m_name= pt.get("name","");
    m_tech= pt.get("tech","");
    m_id= pt.get("id",0);
    m_addr = hamcast::uri(pt.get("addr",""));

    boost::property_tree::ptree pt2;

    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("groups")){
        string key;
        pt2.clear();
        key = v.first.data();
        try{
            pt2 = pt.get_child("groups."+key);
            std::stringstream output;
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt2,w );
            std::string input = output.str();
            group g(input);

            m_groups.push_back(g);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
        }
    }
}

std::pair<string, boost::property_tree::ptree> interface::to_xml(){
    std::stringstream output;
    boost::property_tree::ptree pt;
    try{
        pt.add("name", m_name);
        pt.add("tech", m_tech);
        pt.add("id", m_id);
        pt.add("addr", m_addr.str());

        for(unsigned int i =0; i < m_neighbors.size();i++){
            string arg = m_neighbors.at(i).str();
            pt.add("neighbors.neighbor",arg);
        }

        for(unsigned int i =0; i < m_groups.size();i++){
            pt.add_child("groups.group" + boost::lexical_cast<string>(i), m_groups.at(i).to_xml().second);
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
