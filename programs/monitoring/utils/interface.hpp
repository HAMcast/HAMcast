#ifndef INTERFACE_H
#define INTERFACE_H
#include <string>
#include <vector>

#include "hamcast/hamcast.hpp"
#include "group.hpp"
#include <boost/property_tree/ptree.hpp>

class interface
{

private:
    std::string                 m_name;
    std::string                 m_tech;
    int                         m_id;
    hamcast::uri                m_addr;
    std::vector<group>          m_groups;
    std::vector<hamcast::uri>   m_neighbors;

public:
    /**
      * @brief default constructor
      */
    interface();
    /**
      * @brief creating object from xml
      * @param xml = xml that discribes the object
      */
    interface(std::string& xml);

    /**
      *@brief creates object from property tree
      */

    interface(boost::property_tree::ptree pt);

    /**
      * @brief creating interface object
      * @param name = interface name
      * @param tech = interface technology
      * @param id = interface id
      * @param addr = interface address
      * @param groups = groups managed by this interface
      * @param neigbors = neiborset for this interface
      */
    interface(const std::string& name, const std::string& tech,
                const int& id, const hamcast::uri& addr,
                const std::vector<group> groups,
                const std::vector<hamcast::uri> neighbors) : 
        m_name(name), m_tech(tech), m_id(id), m_addr(addr), 
        m_groups(groups),m_neighbors(neighbors) {}

    /**
      * @brief converting object to xml and boost property tree
      */
    void init(boost::property_tree::ptree pt);

    void init(std::string& xml);

    std::pair<std::string,boost::property_tree::ptree> to_xml();

    inline std::string get_name(){return m_name;}

    inline std::string get_tech(){return m_tech;}

    inline int get_id(){return m_id;}

    inline hamcast::uri get_addr(){return m_addr;}

    inline std::vector<group> get_groups(){return m_groups;}

    inline std::vector<hamcast::uri> get_neighbors(){return m_neighbors;}
};

#endif // INTERFACE_H
