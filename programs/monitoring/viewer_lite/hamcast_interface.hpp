#ifndef HAMCASTINTERFACE_H
#define HAMCASTINTERFACE_H

#include <QMap>
#include <QList>
#include <QTreeWidgetItem>
#include <hamcast/hamcast.hpp>
#include "hamcast_group.hpp"

class hamcast_interface
{
private:
    QString                                  m_name;
    QString                                  m_tech;
    int                                      m_id;
    hamcast::uri                             m_addr;
    QMap<hamcast::uri,hamcast_group>         m_groups;
    QList<hamcast::uri>                      m_neighbors;

public:
    /**
      * @brief creating object from xml
      * @param xml = xml that discribes the object
      */
    hamcast_interface(std::string& xml);

    hamcast_interface(boost::property_tree::ptree);

    hamcast_interface(QString addr,int id,QString name,QString tech);

    /**
      * @brief converting object to xml and boost property tree
      */
    std::pair<std::string,boost::property_tree::ptree> to_xml();

    bool operator==(const hamcast_interface &other) const ;

    inline const QString& get_name() const {return m_name;}

    inline const QString& get_tech() const {return m_tech;}

    inline const int& get_id() const {return m_id;}

    inline const hamcast::uri& get_addr() const {return m_addr;}

    inline const QMap<hamcast::uri,hamcast_group>& get_groups() const {return m_groups;}

    inline const QList<hamcast::uri>& get_neighbors() const {return m_neighbors;}

    inline void set_neighbors(const QList<hamcast::uri>& n){m_neighbors = n;}

    inline void set_groups(const QMap<hamcast::uri,hamcast_group>& g){m_groups = g;}

    inline void set_group(hamcast_group& g){m_groups.insert(g.get_name(),g);}

    inline void set_name(QString & n ){m_name = n;}

    inline void set_tech(QString& t) {m_tech = t;}

    inline void set_id(int& id){m_id = id;}

    inline void set_addr(const hamcast::uri& u){m_addr = u;}

    inline const hamcast_group findGroup(const hamcast::uri& group_id) const {return m_groups.value(group_id);}
};

#endif // HAMCASTINTERFACE_H
