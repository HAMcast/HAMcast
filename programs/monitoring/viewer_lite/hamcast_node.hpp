#ifndef HAMCASTNODE_H
#define HAMCASTNODE_H

#include <QTreeWidget>
#include <QMap>
#include <hamcast_interface.hpp>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSplitter>
#include <QList>
#include <QTreeWidgetItem>

#include <QDebug>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>

class ImageNode;
class hamcast_node
{
private:
    QString                     m_name;
    QList<hamcast_interface>    m_interface;
    bool                        m_img;

public:

     hamcast_node();

     /**
       * @brief creating object from xml
       * @param xml = xml that discribes the object
       */
     hamcast_node(std::string& xml);

     hamcast_node(boost::property_tree::ptree & pt);

     /**
       * @brief creates node object
       * @param name = name or address of the node
       * @param vector<interface> = list of all the interfaces of this node
       */
     hamcast_node(QString name, QList<hamcast_interface> interface,bool img);


     bool operator==(const hamcast_node &other) const ;

     inline const QString& get_name() const{
         return m_name;
     }

     inline const QList<hamcast_interface>& get_interfaces() const{
         return m_interface;
     }

     inline void set_interfaces(QList<hamcast_interface>& interfaces){
        m_interface = interfaces;
     }

     inline const bool& is_img() const{
        return m_img;
     }

     inline void set_name(QString& name){
         m_name = name;
     }

};

#endif // HAMCASTNODE_H
