#include <string>
#include "hamcast/uri.hpp"
#include "hamcast/hamcast.hpp"
#include "hamcast/hamcast_module.h"
#include "hamcast/hamcast_logging.h"
#include "mcpo_connection.h"
#include "instance_handlers.h"
#include <boost/algorithm/string.hpp>

#define MCPO_MAX_SIZE 1280


using hamcast::uri;

 inline mcpo_connection* self(hc_module_instance_t instance)
{
    return reinterpret_cast<mcpo_connection*>(instance);
}



 // only this form <someprotocoll>_<someport> to <someprotocoll>{<someport>}
 string build_to_brackets(string str){
     std::vector<std::string> temp;
     boost::split(temp, str, boost::is_any_of("_"));
     std::string blub="";
     for(int i=0; i<temp.size();i=i+2){
     blub=blub+temp.at(i)+"{"+temp.at(i+1)+"}";
    }
     return blub;
     }

 mcpo_connection *mcpo;

 extern "C" void hc_init(hc_log_fun_t log_fun,
                         hc_module_handle* handle,
                         hc_new_instance_callback_t new_instance_cb,
                         hc_recv_callback_t recv_cb,
                         hc_event_callback_t event_cb,
                         hc_atomic_msg_size_callback_t changed_cb,
                         size_t max_message_size,
                         hc_kvp_list_t* ini_arguments){



     struct InstanceHandlers handlers;

     handlers.kvp=ini_arguments;
     handlers.event_cb=event_cb;
     handlers.recv_cb=recv_cb;
     handlers.log_fun=log_fun;

    hc_set_log_fun(log_fun);

     //extract config infos from kvp_list - same format as in ariba config files needed.


    string endpoint_host="";
    string endpoint_port="";
     string spovnetname="";
     string boots_host="";
     string boots_port="";

     string nodename;
     uint32_t port;

     hc_kvp_list_t* kvp_ptr = ini_arguments;


     while(kvp_ptr) {


         std::vector<std::string> strs;
         boost::split(strs, kvp_ptr->key, boost::is_any_of("."));

         if(strs.size()==1){

               //port
                if(string(kvp_ptr->key)==string("port")){

                       std::string val=kvp_ptr->value;
                           std::stringstream ss;
                          ss << val;

                          for (int n=0; n<val.length(); n++)
                          {
                              ss >> port;
                          }

                          HC_LOG_INFO("found port: "<< port);
                }
                else if(string(kvp_ptr->key)==string("spovnetname")){
                    spovnetname = kvp_ptr->value;

                    HC_LOG_DEBUG("found spovnetname: "<< spovnetname);
                }
                else if(string(kvp_ptr->key)==string("nodename")){

                    nodename=kvp_ptr->value;
                    HC_LOG_INFO("found nodename: "<< nodename);
                }
          /*      else if(string(kvp_ptr->key)==string("endpoint")){

                    string str=build_to_brackets(kvp_ptr->value);
                    endpoint=str;

                   HC_LOG_DEBUG("found local endpoint: "<< endpoint);
                }*/


            }
            else if(strs.size()==2){
		if(string(kvp_ptr->key)==string("endpoint.port")){
			string str=build_to_brackets(kvp_ptr->value);
                    	endpoint_port=str;
                }
                else if(string(kvp_ptr->key)==string("endpoint.host")){

                    string str=build_to_brackets(kvp_ptr->value);
                    endpoint_host=str;

                   HC_LOG_DEBUG("found local endpoint host: "<< endpoint_host);
                }


            }
            else if(strs.size()==3){

             if(strcmp((char*)(strs.at(1)).c_str(),"host")==0){
                boots_host=build_to_brackets(kvp_ptr->value);
                HC_LOG_DEBUG("found bootstrap host: "<< boots_host);

             }else if(strcmp((char*)(strs.at(1)).c_str(),"port")==0){
                 boots_port=build_to_brackets(kvp_ptr->value);
                 HC_LOG_DEBUG("found bootstrap port : "<< boots_port);
             }


            }        kvp_ptr = kvp_ptr->next;

     }




     mcpo= new mcpo_connection(handlers, nodename, spovnetname+"{"+boots_host+ boots_port+ "};", endpoint_host+";"+endpoint_port, spovnetname, port);


     // build kvp list
     string name_key ("if_name");
     string name_val ("mcpo");
     string addr_key ("if_addr");
     string addr_val ("ariba://"+mcpo->getNodeID());
     string tech_key ("if_tech");
     string tech_val ("ariba");

     hc_kvp_list_t name;
     name.key = name_key.c_str();
     name.value = name_val.c_str();
     name.next = 0;

     hc_kvp_list_t addr;
     addr.key = addr_key.c_str();
     addr.value = addr_val.c_str();
     addr.next = &name;

     hc_kvp_list_t tech;
     tech.key = tech_key.c_str();
     tech.value = tech_val.c_str();
     tech.next = &addr;


     hc_module_instance_handle_t hdl = new_instance_cb(mcpo, handle, &tech, MCPO_MAX_SIZE);
     mcpo->set_handle(hdl);
    mcpo->set_max_msg_size(MCPO_MAX_SIZE);






}


 extern "C" int hc_join (hc_module_instance_t instance_ptr, const hc_uri_t *group_uri, const char *group_uri_str){
    HC_LOG_DEBUG("entered hc_join with port: " << group_uri->port_as_int());
    if(!(group_uri->empty())){
    self(instance_ptr)->join(group_uri, group_uri_str);
    }else {
        HC_LOG_ERROR("not joining group: "<< group_uri_str << " - invalid URI");
        return HC_INVALID_URI;
    }
    return HC_SUCCESS;

}


 extern "C" int hc_leave(hc_module_instance_t instance_ptr, const hc_uri_t *group_uri, const char *group_uri_str){

     self(instance_ptr)->leave(group_uri, group_uri_str);
return HC_SUCCESS;
 }


 extern "C" int hc_sendto(hc_module_instance_t instance_ptr, const void *buf, int len, unsigned char ttl, const hc_uri_t *group_uri, const char *group_uri_str){

     HC_LOG_DEBUG("try to send message...");
    self(instance_ptr)->sendToGroup(buf, len, group_uri);
     return HC_SUCCESS;
 }

 //TODO: DUMMY
 extern "C" hc_uri_result_t hc_map(hc_module_instance_t instance_ptr,
                        const hc_uri_t* group_uri, const char* uri_str){
    return hamcast::create_uri_result(*group_uri);

   //  return self(instance_ptr)->mcpo_map(group_uri,uri_str);
 }

extern "C" hc_uri_list_t hc_group_set(hc_module_instance_t instance_ptr){
    std::vector<hamcast::uri>uris=self(instance_ptr)->get_groups();

   return hamcast::create_uri_list(uris);
 }

extern "C" hc_uri_list_t hc_neighbor_set(hc_module_instance_t instance_ptr){

    std::vector<hamcast::uri>uris=self(instance_ptr)->get_neighbors();

   return hamcast::create_uri_list(uris);

 }

extern "C" hc_uri_list_t hc_children_set(hc_module_instance_t instance_ptr,
                               const hc_uri_t* uri, const char*){

    std::vector<hamcast::uri>uris=self(instance_ptr)->get_children(uri);
    HC_LOG_TRACE("got vector from mcpoinstance- size"<<uris.size());


   return hamcast::create_uri_list(uris);
 }


extern "C" hc_uri_list_t hc_parent_set(hc_module_instance_t instance_ptr,
                             const hc_uri_t* uri, const char*){
     hc_uri_list_t result;

     std::vector<hamcast::uri> uris=self(instance_ptr)->get_parent(uri);

     for(int i=0; i<uris.size();i++){
         HC_LOG_TRACE("parent: "<< uris.at(i));
     }
     HC_LOG_TRACE("got vector from mcpoinstance- size"<<uris.size());
     return hamcast::create_uri_list(uris);
;

    return result; }

extern "C" int hc_designated_host(hc_module_instance_t instance_ptr,
                        const hc_uri_t*, const char*){
     return HC_IS_NOT_A_DESIGNATED_HOST;
 }

extern "C" void hc_shutdown(){
    HC_LOG_TRACE("trying to shutdown...");
    mcpo->hc_shutdown();
}


