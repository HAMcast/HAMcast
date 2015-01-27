#include "hamcast_node.hpp"
#include <QDebug>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

hamcast_node::hamcast_node(std::string& xml){
    std::stringstream input(xml);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    m_name= QString::fromStdString(pt.get("name",""));
    m_img = pt.get("img",false);
    boost::property_tree::ptree pt2;

    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("interfaces")){
        std::string key;
        pt2.clear();
        key = v.first.data();
        try{
            pt2 = pt.get_child("interfaces."+key);
            hamcast_interface i(pt2);
            m_interface.push_back(i);
        }
        catch(std::exception& e){
            std::cout << e.what() << std::endl;
            qDebug() << "hamcast node exception";
            continue;
        }
    }
}

hamcast_node::hamcast_node(boost::property_tree::ptree & pt){


    m_name= QString::fromStdString(pt.get("name",""));
    m_img = pt.get("img",false);
    boost::property_tree::ptree pt2;
    try{
        BOOST_FOREACH(boost::property_tree::ptree::value_type &v,pt.get_child("interfaces")){
            std::string key;
            pt2.clear();
            key = v.first.data();

            pt2 = pt.get_child("interfaces."+key);
            hamcast_interface i (pt2);
            m_interface.push_back(i);
        }
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
}


hamcast_node::hamcast_node()
{
}


hamcast_node::hamcast_node(QString name, QList<hamcast_interface> interface,bool img)
    : m_name(name),m_interface(interface),m_img(img)
{
}

bool hamcast_node:: operator==(const hamcast_node &other) const {
    int comp_1;
    comp_1 = m_name.compare(other.get_name());
    if(comp_1 !=0 || m_interface!= other.get_interfaces()|| m_img != other.is_img()) return false;
    return true;
}
