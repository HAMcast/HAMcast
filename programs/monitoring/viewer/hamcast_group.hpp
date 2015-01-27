#ifndef HAMCAST_GROUP_HPP
#define HAMCAST_GROUP_HPP

#include <hamcast/hamcast.hpp>
#include <QList>
#include <boost/property_tree/ptree.hpp>

class hamcast_group
{
private:
    hamcast::uri              m_name;
    hamcast::uri              m_parent;
    QList<hamcast::uri>       m_children;

public:


   hamcast_group();

  /**
  * @brief create group object
  * @param name = group uri
  * @param parent = uri of the parent for this group
  * @param vector<uri> children = vector over children for this group
  */

    hamcast_group(hamcast::uri& name, hamcast::uri& parent, QList<hamcast::uri> children);

    /**
      * @brief creates a group object from xml
      * @param xml that discribes the object
      */
    hamcast_group(std::string& xml);

    hamcast_group(boost::property_tree::ptree pt);


    /**
      * @brief compare funktion for QMap compare
      */
    bool operator==(const hamcast_group &other) const ;

    inline const hamcast::uri get_name() const {
        return m_name;
    }

    inline const hamcast::uri& get_parent() const {
        return m_parent;
    }

    inline const QList<hamcast::uri>& get_children() const {
        return m_children;
    }

    inline void set_name(hamcast::uri& name){
        m_name = name;
    }

    inline void set_parent(hamcast::uri& parent){
        m_parent = parent;
    }

    inline void set_children(QList<hamcast::uri>& children){
        m_children = children;
    }

    inline void set_child(hamcast::uri child){m_children.append(child);}
};

#endif // HAMCAST_GROUP_HPP
