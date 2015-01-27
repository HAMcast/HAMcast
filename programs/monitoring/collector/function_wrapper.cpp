#include "function_wrapper.hpp"
#include "node.hpp"
#include "interface.hpp"
#include "group.hpp"
#include "boost/property_tree/ptree.hpp"
#include <algorithm>

function_wrapper::function_wrapper(collector& collector)
    : m_collector(collector)
{
    register_function("/group_list",(function_call) &function_wrapper::group_list);
    register_function("/node_list",(function_call) &function_wrapper::node_list);
    register_function("/group_data",(function_call) &function_wrapper::group_data);
    register_function("/node_data",(function_call) &function_wrapper::node_data);
    register_function("/group_tree",(function_call) &function_wrapper::group_tree);
}

std::string function_wrapper::group_list(std::vector<std::string> args){
    if(args.size() > 0){
        return std::string();
    }
    else{
        std::vector<std::string> tmp;
        std::vector<std::string>::iterator finder;
        std::stringstream output;
        boost::property_tree::ptree pt;
        pt.add("method","/group_list");
        std::map<std::string,boost::shared_ptr<node> > node_list = m_collector.get_node_list();
        std::map<std::string,boost::shared_ptr<node> >::const_iterator itr;
        for(itr = node_list.begin(); itr != node_list.end(); ++itr){
            boost::shared_ptr<node> n = (*itr).second;
            std::vector<interface> interfaces = n->get_interfaces();
            for(int i =0; i < interfaces.size();i++){
                interface inter = interfaces.at(i);
                std::vector<group> groups = inter.get_groups();
                for(int g =0; g < groups.size();g++){

                    group grp = groups.at(g);
                    finder = find(tmp.begin(),tmp.end(),grp.get_name().str());
                    if(finder == tmp.end()){
                        pt.add("groups.group",grp.get_name());
                        tmp.push_back(grp.get_name().str());
                    }
                }
            }
        }
        try{
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
        }
        catch (std::exception& e){
            std::cout << e.what() << std::endl;
        }
        return output.str();
    }
}

std::string function_wrapper::node_list(std::vector<std::string> args){
    if(args.size() > 0){
        return std::string();
    }
    else{
        std::stringstream output;
        boost::property_tree::ptree pt;
        pt.add("method","/node_list");
        std::map<std::string,boost::shared_ptr<node> > node_list = m_collector.get_node_list();
        std::map<std::string,boost::shared_ptr<node> >::const_iterator itr;
        for(itr = node_list.begin(); itr != node_list.end(); ++itr){
            pt.add("nodes.node",(*itr).first);
        }
        try{
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
        }
        catch (std::exception& e){
            std::cout << e.what() << std::endl;
        }
        return output.str();
    }
}

std::string function_wrapper::group_data(std::vector<std::string> args){
    if(args.size() < 1){
        return std::string();
    }
    else{
        std::vector<std::string> tmp;
        std::vector<std::string>::iterator finder;
        hamcast::uri comp_group(args.at(0));
        std::stringstream output;
        boost::property_tree::ptree pt;
        pt.add("method","/group_data");
        pt.add("group.name",comp_group.str());
        std::map<std::string,boost::shared_ptr<node> > node_list = m_collector.get_node_list();
        std::map<std::string,boost::shared_ptr<node> >::const_iterator itr;
        for(itr = node_list.begin(); itr != node_list.end(); ++itr){
            boost::shared_ptr<node> n = (*itr).second;
            std::vector<interface> interfaces = n->get_interfaces();
            for(int i =0; i < interfaces.size();i++){
                interface inter = interfaces.at(i);
                std::vector<group> groups = inter.get_groups();
                for(int g =0; g < groups.size();g++){
                    group grp = groups.at(g);
                    if(grp.get_name() == comp_group)
                        finder = find(tmp.begin(),tmp.end(),(*itr).first);
                    if(finder == tmp.end()){
                        tmp.push_back((*itr).first);
                        pt.add("group.member",(*itr).first);
                    }
                }
            }
        }
        try{
            boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
            boost::property_tree::xml_parser::write_xml( output, pt,w );
        }
        catch (std::exception& e){
            std::cout << e.what() << std::endl;
        }
        return output.str();
    }
}

std::string function_wrapper::node_data(std::vector<std::string> args){
    if(args.size() < 1){
        return std::string();
    }
    else{
        std::string node_name(args.at(0));
        std::map<std::string,boost::shared_ptr<node> > node_list = m_collector.get_node_list();

        std::map<std::string,boost::shared_ptr<node> >::const_iterator itr = node_list.find(node_name);
        if(itr != node_list.end()){
            boost::shared_ptr<node> found_node = (*itr).second;
            return found_node->to_xml().first;
        }
        return std::string();
    }
}

std::string function_wrapper::group_tree(std::vector<std::string> args){

    //TODO kann man noch optimieren

    if(args.size() < 1){
        return std::string();
    }
    else{
        try{

            bool dont_ip = true;
            hamcast::uri group_name(args.at(0));
            std::stringstream output;
            boost::property_tree::ptree pt;
            boost::shared_ptr<node> tunnel_img;
            pt.add("method","/group_tree");
            std::map<std::string,boost::shared_ptr<node> > node_list = m_collector.get_node_list();
            std::map<std::string,boost::shared_ptr<node> >::const_iterator itr;
            int edge_counter =0;
            std::vector<std::pair<boost::shared_ptr<node>,hamcast::uri> > ip_nodes;
            std::vector<std::pair<std::pair<boost::shared_ptr<node>, std::string > ,hamcast::uri> > img_nodes;
            pt.add("name",group_name.str());
            for(itr = node_list.begin(); itr != node_list.end(); itr++){
                boost::shared_ptr<node> n = (*itr).second;
                std::vector<interface> interfaces = n->get_interfaces();
                for(int i =0; i < interfaces.size();i++){
                    interface inter = interfaces.at(i);
                    std::vector<group> groups = inter.get_groups();
                    bool dont_tunnel =false;
                    for(int g =0; g < groups.size();g++){
                        group grp = groups.at(g);
                        if(grp.get_name()== group_name){
                            if(n->is_img()){
                                if(inter.get_tech()== "IPv4"){
                                    img_nodes.push_back(std::pair< std::pair< boost::shared_ptr<node>, std::string > ,hamcast::uri >(std::pair<boost::shared_ptr<node>, std::string >(n,inter.get_tech()),inter.get_addr()));
                                }
                                if(inter.get_tech()== "tunnel") {
                                    tunnel_img = n;
                                }
                            }
                            else{
                                if(inter.get_tech()== "IPv4"){
                                    ip_nodes.push_back(std::pair<boost::shared_ptr<node>,hamcast::uri>(n,inter.get_addr()));
                                }
                            }
                            if(inter.get_tech()=="tunnel"){
                                for(int t1=0; t1 < interfaces.size(); t1++){
                                    interface p_inter = interfaces.at(t1);
                                    if(p_inter.get_tech() == "IPv4"){
                                        for (int g1 =0; g1 < p_inter.get_groups().size();g1++) {
                                            group g = p_inter.get_groups().at(g1);
                                            if(g.get_name() == group_name){
                                                if(g.get_children().size() > 0 ){
                                                    for (int ig=0;ig < g.get_children().size();ig++){
                                                        hamcast::uri u = g.get_children().at(ig);
                                                        if(u.str().compare("ip://0.0.0.0") != 0){
                                                            dont_tunnel = true;
                                                            dont_ip = false;
                                                        }

                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if(!dont_tunnel){
                                std::vector<hamcast::uri> children = grp.get_children();
                                for (int c =0; c < children.size();c++){
                                    std::map<hamcast::uri,std::string> daemon_map =m_collector.get_deamon_map();
                                    std::map<hamcast::uri,std::string>::const_iterator dmap_itr = daemon_map.find(children.at(c));
                                    if(dmap_itr != daemon_map.end()){
                                        pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".from",(*itr).first);
                                        pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".to",(*dmap_itr).second);
                                        pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".tech",inter.get_tech());
                                        edge_counter++;
                                    }
                                }
                            }
                            dont_tunnel = false;
                        }
                    }
                }
            }

            for(int ip =0; ip < ip_nodes.size();ip++){
                std::string addr1 = ip_nodes.at(ip).second.str();
                int lp =-1;
                int root = -1;
                for(int img =0; img < img_nodes.size();img++){
                    std::string addr2 = img_nodes.at(img).second.str();
                    int tmp = lcp(addr1,addr2);
                    if(tmp > lp){
                        lp = tmp;
                        root = img;
                    }
                }
                if(root !=-1){
                    std::map<hamcast::uri,std::string> daemon_map =m_collector.get_deamon_map();
                    if(lp < 6){
                        std::map<hamcast::uri,std::string>::const_iterator dmap_itr = daemon_map.find(hamcast::uri(addr1));
                        if(dmap_itr != daemon_map.end()){
                            pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".from","N/A");
                            pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".to",(*dmap_itr).second);
                            edge_counter++;
                        }
                    }
                    else{

                        if(img_nodes.at(root).first.first == tunnel_img && dont_ip){

                        }
                        else{
                            std::map<hamcast::uri,std::string>::const_iterator dmap_itr = daemon_map.find(img_nodes.at(root).second);
                            if(dmap_itr != daemon_map.end()){
                                pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".from",(*dmap_itr).second);
                            }
                            dmap_itr = daemon_map.find(hamcast::uri(addr1));
                            if(dmap_itr != daemon_map.end()){
                                pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".to",(*dmap_itr).second);
                            }
                            pt.add("edges.edge"+boost::lexical_cast<std::string>(edge_counter)+".tech",img_nodes.at(root).first.second);
                            edge_counter++;
                        }
                    }
                }
            }
            try{
                boost::property_tree::xml_parser::xml_writer_settings<char> w(' ', 2);
                boost::property_tree::xml_parser::write_xml( output, pt,w );
            }
            catch (std::exception& e){
                std::cout << e.what() << std::endl;
            }
            return output.str();
        }
        catch(std::exception &e){
            std::cout << e.what() << std::endl;
            std::cout << "TREE CALL" << std::endl;
        }
    }

}

int function_wrapper::lcp(std::string& n1,std::string& n2){
    int i =0;
    for(; i < n1.length();i++){
        std::string tmp =n2.substr(0,i);
        int res = n1.compare(0,i,tmp);
        if(res ==0){
            continue;
        }
        else{
            return i;
        }
    }
    return i;
}
