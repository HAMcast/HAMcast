#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include <boost/property_tree/ptree.hpp>

#include "hamcast/hamcast.hpp"

class group
{
public:

    /**
     * @brief create group object
     * @param name = group uri
     * @param parent = uri of the parent for this group
     * @param vector<uri> children = vector over children for this group
     */
    group(hamcast::uri& name, hamcast::uri& parent, std::vector<hamcast::uri> children) : 
        m_name(name), m_parent(parent), m_children(children)
{
}

    /**
      * @brief creates a group object from xml
      * @param xml that discribes the object
      */
    group(std::string& xml);

    /**
      * @brief creates a group object from boost property tree
      * @param property tree that discribes the object
      */
    group(boost::property_tree::ptree pt);

    /**
      * @brief converting object to xml
      */
    std::pair<std::string,boost::property_tree::ptree> to_xml();

    inline hamcast::uri get_name(){
        return m_name;
    }

    inline hamcast::uri get_parent(){
        return m_parent;
    }

    inline std::vector<hamcast::uri> get_children(){
        return m_children;
    }


private:
    hamcast::uri              m_name;
    hamcast::uri              m_parent;
    std::vector<hamcast::uri> m_children;
};

#endif // GROUP_H
