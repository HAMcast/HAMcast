#ifndef COLLECTOR_HPP
#define COLLECTOR_HPP

#include <map>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>

#include "hamcast/hamcast.hpp"
#include "node.hpp"

typedef boost::shared_mutex Lock;
typedef boost::unique_lock< boost::shared_mutex > WriteLock;
typedef boost::shared_lock< boost::shared_mutex >  ReadLock;


class collector
{
private:

    std::map<std::string,boost::shared_ptr<node> > node_list;
    std::map<hamcast::uri,std::string> daemon_map;
    Lock list_lock;

public:

    collector() {}

    /**
      *@brief delivers a node to the collector
      *@param node = boost shared_ptr to the node object
      *@param daemon_id
      */
    void deliver_node(boost::shared_ptr<node> node, std::string daemon_id);

    /**
     * @brief removes a node from the collector
     */
    void remove_node(std::string daemon_id);

    /**
     * @brief returns a list of all delivered nodes
     */
    inline std::map<std::string,boost::shared_ptr<node> > get_node_list(){
        ReadLock r_lock(list_lock);
        return node_list;
    }

    inline std::map<hamcast::uri,std::string>& get_deamon_map(){
        return daemon_map;
    }

    std::string to_string();
};

#endif // COLLECTOR_HPP
