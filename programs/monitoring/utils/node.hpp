#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "interface.hpp"

class node
{
private:
    std::string            m_name;
    std::vector<interface> m_interface;
    bool                   m_img;

public:
    /**
      * @brief creating object from xml
      * @param xml = xml that discribes the object
      */
    node(std::string& xml);

    /**
      * @brief creates node object
      * @param name = name or address of the node
      * @param vector<interface> = list of all the interfaces of this node
      */
    node(std::string name,std::vector<interface> interfaceArg,bool img);

    /**
      * @brief creates a XML output and boost property tree for this object
      */

    std::pair<std::string,boost::property_tree::ptree> to_xml();


    inline std::string get_name(){
        return m_name;
    }

    inline std::vector<interface> get_interfaces(){
        return m_interface;
    }

    inline bool is_img() {
        return m_img;
    }
};

#endif // NODE_H
