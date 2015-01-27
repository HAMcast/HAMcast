/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <vector>

#include "hamcast/hamcast_logging.h"
#include "discovery/mc_tables.hpp"

using std::string;
using std::stringstream;
using std::vector;
using std::ifstream;

mc_tables::mc_tables(): m_addr_family(-1){
}

string mc_tables::int2str(int x){
     HC_LOG_TRACE("");
     char buf[11]; // max int stellen
     snprintf(buf, sizeof(buf), "%d", x );
     return string(buf);
}

int mc_tables::str2int(string x){
     HC_LOG_TRACE("");
     std::stringstream stream;
     int res;
     stream << x;
     stream >> res;
     return res;
}


void mc_tables::init_tables(int addrFamily){
     m_addr_family = addrFamily;
}

int mc_tables::hexChar_To_int(char x){
     HC_LOG_TRACE("");

     if(x >= '0' && x <='9'){
          return x - '0';
     }else if(x >= 'a' && x <='f'){
          return x - 'a' + 10;
     }else if(x >= 'A' && x <='F'){

          return x - 'A' + 10;
     }else{
          return -1;
     }
}

addr_storage mc_tables::hexCharAddr_To_ipFormart(string& ipAddr, int addrFamily){
     HC_LOG_TRACE("");

     std::ostringstream normal_ip;

     if(addrFamily == AF_INET){ //fill normal_ip
          //"FB0000E0" ==> "251.0.0.224"

          int z1, z2;

          for(unsigned int i=ipAddr.length(); i > 2; i-=2){
               z1=hexChar_To_int(ipAddr.at(i-2));
               z2=hexChar_To_int(ipAddr.at(i-1));
               normal_ip <<(z1*16+z2) << ".";
          }

          z1=hexChar_To_int(ipAddr.at(0));
          z2=hexChar_To_int(ipAddr.at(1));
          normal_ip <<(z1*16+z2);

          return addr_storage(normal_ip.str());
     }else if(addrFamily == AF_INET6){ //fill normal ip
          if(ipAddr.find_first_of(':')==std::string::npos){
               //"ff020000000000000000000000000001" ==> "ff02:0000:0000:0000:0000:0000:0000:0001"
               for(unsigned int i = 0; i<ipAddr.length(); i++){
                    normal_ip << ipAddr.at(i);
                    if((i+1)%4 == 0 && i<ipAddr.length()-1){
                         normal_ip << ":";
                    }
               }

               return addr_storage(normal_ip.str());
          }else{
               return addr_storage(ipAddr);
          }

     }else{
          HC_LOG_ERROR("wrong addrFamily: " << addrFamily);
          return addr_storage();
     }

}

void mc_tables::trim(char* str, unsigned int size){
     //trim first white spaces
     int countv=0;
     unsigned int i=0;
     while(i< size){
          if(str[i] == ' '){
               countv++;
          }else{
               if(countv>0){
                    memcpy((void*)str,(void*)&str[countv], size-countv);
                    memset((void*)&str[size-countv],0,countv);
                    break;
               }else{
                    break;
               }
          }
          i++;
     }
     //trim last white spaces

     //find last usefull char
     i=0;
     int lastchar=0;
     while(i< size){
          if(str[i]==0){
               lastchar=i-1;
               break;
          }
          i++;
     }
     //delete all last spaces
     while(lastchar >= 0){

          if(str[lastchar] == ' '){
               str[lastchar] = 0;
          }else{
               break;
          }

          lastchar--;
     }

}

//#######################
//##-- joined groups --##
//#######################

bool mc_tables::refresh_joined_groups(){
     HC_LOG_TRACE("");

     m_mgroup_map.clear();

     ifstream file;
     char cstr[MAX_N_LINE_LENGTH];
     string tmp;
     string device;

     if(m_addr_family == AF_INET){
          //--IPv4--
          //fill m_mgroup_map
          //file.open("/home/barbie/Dropbox/HAMcast/Recherche/igmp");
          file.open(JOINED_GROUP_PATH_V4);
          if(!file){
               HC_LOG_ERROR("can't open file: " << JOINED_GROUP_PATH_V4);
               return false;
          }else{
               int status=0;  //0 = status line | 1 = counter line
               int nline=0;
               int ipCounter;
               MGroup_value ipAddr;

               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){
                   std::stringstream strline;
                    strline << cstr;

                    if(nline==0){
                         //init line nothing to do
                    }else if(status == 0){
                         strline >> tmp; //Idx
                         strline >> device; //Device
                         strline >> tmp; //:
                         strline >> ipCounter; //Count
                         status=1;
                    }else if(status == 1){
                         strline >> tmp; //Group
                         ipAddr.push_back(hexCharAddr_To_ipFormart(tmp,AF_INET));

                         ipCounter--;
                         if(ipCounter<=0){
                              status = 0;
                              m_mgroup_map.insert(MGroup_pair(device,ipAddr));
                              ipAddr.clear();
                         }
                    }
                    nline++;
                    file.getline(cstr,sizeof(cstr));
               }
          }
          file.close();
          file.clear();
          return true;

     }else if(m_addr_family == AF_INET6){
          //--IPv6--
          //fill m_mgroup_map
          //file.open("/home/barbie/Dropbox/HAMcast/Recherche/igmp6");
          file.open(JOINED_GROUP_PATH_V6);
          if(!file){
               HC_LOG_ERROR("can't open file: " << JOINED_GROUP_PATH_V6);
               return false;
          }else{
               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){
                   std::stringstream strline;
                    strline << cstr;

                    strline >> tmp; //interface counter
                    strline >> device; //Device

                    MGroup_value* ipAddr= &m_mgroup_map[device];

                    strline >> tmp; //ipv6 addr
                    ipAddr->push_back(hexCharAddr_To_ipFormart(tmp,AF_INET6));

                    file.getline(cstr,sizeof(cstr));
               }
               file.close();


          }
          return true;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}

unsigned int mc_tables::get_joined_groups_count(string& ifName){
     HC_LOG_TRACE("");

     MGroup_value* tmp = &m_mgroup_map[ifName];
     return tmp->size();
}

addr_storage mc_tables::get_joined_group(string& ifName, unsigned int index){
     HC_LOG_TRACE("");

     MGroup_value* tmp = &m_mgroup_map[ifName];
     return (*tmp)[index];
}

//###############
//##-- vifs -- ##
//###############
/*
bool mc_tables::refresh_vifs(){
     HC_LOG_TRACE("");

     m_mr_vif.clear();

     ifstream file;
     char cstr[MAX_N_LINE_LENGTH];

     if(m_addr_family == AF_INET){
          //--IPv4--
          file.open(VIF_PATH_V4);
          if(!file){
               HC_LOG_ERROR("can't open file: " << VIF_PATH_V4);
               return false;
          }else{
               int status=0;  //0 = status line | 1 = counter line
               struct mr_vif vif_tmp;
               string str;

               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){
                   std::stringstream strline;
                    strline << cstr;

                    if(status == 0){
                         status=1;
                    }else if(status == 1){
                         vif_tmp.addr_family = AF_INET;
                         strline >> vif_tmp.vifi;
                         strline >> vif_tmp.ifname;
                         strline >> vif_tmp.bytesIn;
                         strline >> vif_tmp.pktsIn;
                         strline >> vif_tmp.bytesOut;
                         strline >> vif_tmp.pktsOut;
                         strline >> vif_tmp.flags;
                         if(vif_tmp.flags & VIFF_USE_IFINDEX){
                              strline >> vif_tmp.lcl_index;
                         }else{
                              strline >> str;
                              vif_tmp.lcl_addr = hexCharAddr_To_ipFormart(str,AF_INET);
                         }

                         if(vif_tmp.flags & VIFF_TUNNEL){
                              strline >> str;
                              vif_tmp.remote = hexCharAddr_To_ipFormart(str,AF_INET);
                         }

                         m_mr_vif.push_back(vif_tmp);
                    }
                    file.getline(cstr,sizeof(cstr));
               }
          }
          file.close();
          file.clear();
          return true;

     }else if(m_addr_family == AF_INET6){
          //--IPv6--
          file.open(VIF_PATH_V6);
          if(!file){
               HC_LOG_ERROR("can't open file: " << VIF_PATH_V6);
               return false;
          }else{
               int status=0;  //0 = status line | 1 = counter line
               struct mr_vif vif_tmp;

               string str;

               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){
                   std::stringstream strline;
                    strline << cstr;

                    if(status == 0){
                         status=1;
                    }else if(status == 1){
                         vif_tmp.addr_family = AF_INET6;
                         strline >> vif_tmp.vifi;
                         strline >> vif_tmp.ifname;
                         strline >> vif_tmp.bytesIn;
                         strline >> vif_tmp.pktsIn;
                         strline >> vif_tmp.bytesOut;
                         strline >> vif_tmp.pktsOut;
                         strline >> vif_tmp.flags;

                         m_mr_vif.push_back(vif_tmp);
                    }
                    file.getline(cstr,sizeof(cstr));
               }
          }
          file.close();
          file.clear();
          return true;

     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}
*/
unsigned int mc_tables::get_vifs_count(){
     HC_LOG_TRACE("");
     return m_mr_vif.size();
}

struct mr_vif mc_tables::get_vif(unsigned int index){
     HC_LOG_TRACE("index: " << index);
     return m_mr_vif[index];
}


//################
//##-- routes --##
//################

bool mc_tables::refresh_routes(){
     HC_LOG_TRACE("");

     m_mr_vif.clear();

     ifstream file;
     char cstr[MAX_N_LINE_LENGTH];

     if(m_addr_family == AF_INET || m_addr_family == AF_INET6){
          //--IPv4--
          string path;
          if(m_addr_family == AF_INET){
               path = MR_CACHE_PATH_V4;
          }else{
               path = MR_CACHE_PATH_V6;
          }

          file.open(path.c_str());
          if(!file){
               HC_LOG_ERROR("can't open file: " << path);
               return false;
          }else{
               int status=0;  //0 = status line | 1 = counter line
               struct mr_cache cache_tmp;
               string str;

               file.getline(cstr,sizeof(cstr));
               trim(cstr,sizeof(cstr));

               while(!file.eof()){
                    stringstream strline;
                    strline << cstr;

                    if(status == 0){
                         status=1;
                    }else if(status == 1){
                         cache_tmp.addr_family = m_addr_family;

                         strline >> str;
                         cache_tmp.group = hexCharAddr_To_ipFormart(str,m_addr_family);

                         strline >> str;
                         cache_tmp.origin = hexCharAddr_To_ipFormart(str,m_addr_family);

                         strline >> cache_tmp.i_if;
                         strline >> cache_tmp.pkts;
                         strline >> cache_tmp.bytes;
                         strline >> cache_tmp.wrong;

                         while (!strline.eof()){
                              stringstream substr;
                              int o_if=0;

                              strline >> str;
                              str = str.substr(0, str.find(":"));

                              substr << str;
                              substr >> o_if;

                              cache_tmp.o_if.push_back(o_if);
                         }

                         m_mr_cache.push_back(cache_tmp);
                    }
                    file.getline(cstr,sizeof(cstr));
                    trim(cstr,sizeof(cstr));

               }
          }
          file.close();
          file.clear();
          return true;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}

unsigned int mc_tables::get_routes_count(){
     HC_LOG_TRACE("");
     return m_mr_cache.size();
}

struct mr_cache mc_tables::get_route(unsigned int index){
     HC_LOG_TRACE("");
     return m_mr_cache[index];
}

//###############
//##-- snmp6 --##
//###############

bool mc_tables::refresh_snmp6(){
     HC_LOG_TRACE("");

     if(m_addr_family != AF_INET6){
          return false;
     }

     m_snmp6_map.clear();

     ifstream file;
     char cstr[MAX_N_LINE_LENGTH];

     DIR *hdir;
     struct dirent *entry;
     string device_name;
     hdir = opendir("/proc/net/dev_snmp6");

     entry = readdir(hdir);
     while(entry){
          stringstream path;
          device_name = entry->d_name;
          if(device_name.compare(".") == 0 || device_name.compare("..") == 0){
               entry = readdir(hdir);
               continue;
          }

          path << SNMP6_PATH << "/" << device_name;

          file.open(path.str().c_str());
          if(!file){
               HC_LOG_ERROR("can't open file: " << path.str());
          }else{

               SNMP6_value snmp_value = get_snmp6_empty_struct();

               string comp_str;

               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){

                    stringstream strline;
                    strline << cstr;

                    strline >> comp_str;
                    if(comp_str.compare("Icmp6InGroupMembQueries") == 0){
                         strline >> snmp_value.Icmp6InGroupMembQueries;
                    }else if(comp_str.compare("Icmp6OutGroupMembQueries") == 0){
                         strline >> snmp_value.Icmp6OutGroupMembQueries;
                    }else if(comp_str.compare("Icmp6InGroupMembResponses") == 0){
                         strline >> snmp_value.Icmp6InGroupMembResponses;
                    }else if(comp_str.compare("Icmp6OutGroupMembResponses") == 0){
                         strline >> snmp_value.Icmp6OutGroupMembResponses;
                    }else if(comp_str.compare("Icmp6InMLDv2Reports") == 0){
                         strline >> snmp_value.Icmp6InMLDv2Reports;
                    }else if(comp_str.compare("Icmp6OutMLDv2Reports") == 0){
                         strline >>snmp_value.Icmp6OutMLDv2Reports;
                    }

                    file.getline(cstr,sizeof(cstr));
               }

               m_snmp6_map.insert(SNMP6_pair(device_name,snmp_value));

          }
          file.close();
          file.clear();

          entry = readdir(hdir);
     }
     closedir(hdir);

     return true;
}

struct snmp6 mc_tables::get_snmp6_empty_struct(){
     HC_LOG_TRACE("");

     SNMP6_value snmp_value;
     snmp_value.Icmp6InGroupMembQueries=-1;
     snmp_value.Icmp6OutGroupMembQueries=-1;
     snmp_value.Icmp6InGroupMembResponses=-1;
     snmp_value.Icmp6OutGroupMembResponses=-1;
     snmp_value.Icmp6InMLDv2Reports=-1;
     snmp_value.Icmp6OutMLDv2Reports=-1;
     return snmp_value;
}

struct snmp6 mc_tables::get_snmp6(string if_name){
     HC_LOG_TRACE("");

     if(m_snmp6_map.find(if_name) != m_snmp6_map.end()){
          return m_snmp6_map[if_name];
     }else{
          return get_snmp6_empty_struct();
     }
}

vector<string> mc_tables::get_snmp6_all_interfaces(){
     HC_LOG_TRACE("");

     vector<string> tmp;

     SNMP6_map::iterator item= m_snmp6_map.begin();

     while(item != m_snmp6_map.end()){
          tmp.push_back((*item).first);
          item++;
     }

     return tmp;
}

//####################
//##-- igmp table --##
//####################

bool mc_tables::refresh_igmp_table(){
     HC_LOG_TRACE("");

     m_igmp_table.clear();

     ifstream file;
     char cstr[MAX_N_LINE_LENGTH];
     string tmp;
     string device;

     if(m_addr_family == AF_INET){
          //--IPv4--
          //fill m_mgroup_map
          file.open(JOINED_GROUP_PATH_V4);
          if(!file){
               HC_LOG_ERROR("can't open file: " << JOINED_GROUP_PATH_V4);
               return false;
          }else{
               int status=0;  //0 = status line | 1 = counter line
               int nline=0;
               int ipCounter=0;

               struct igmp_dev dev;
               struct igmp_group gr;

               file.getline(cstr,sizeof(cstr));
               while(!file.eof()){
                    stringstream strline;
                    strline << cstr;

                    if(nline==0){
                         //init line nothing to do
                    }else if(status == 0){
                         strline >> dev.index; //Idx
                         strline >> dev.if_name; //Device
                         strline >> tmp; //:
                         strline >> dev.g_count; //Count
                         ipCounter = dev.g_count;
                         strline >> tmp;
                         dev.querier_version = str2int(tmp.substr(1,tmp.size()));
                         status=1;
                    }else if(status == 1){
                         strline >> tmp; //Group
                         gr.group = hexCharAddr_To_ipFormart(tmp,AF_INET);
                         strline >> gr.users;
                         strline >> tmp; //timer 0:00000000
                         gr.timer_run = tmp.substr(0,1).compare("1") ==0? true : false;
                         stringstream tmp_stream;
                         tmp_stream << "0x" << tmp.substr(2,tmp.size());
                         tmp_stream >> tmp;
                         gr.timer = ((int)strtod(tmp.c_str(),NULL))*10;
                         strline >> gr.reporter;

                         dev.groups.push_back(gr);
                         ipCounter--;
                         if(ipCounter<=0){
                              status = 0;
                              m_igmp_table.push_back(dev);
                              dev.groups.clear();
                         }
                    }
                    nline++;
                    file.getline(cstr,sizeof(cstr));
               }
          }
          file.close();
          file.clear();
          return true;
     }else{
          HC_LOG_ERROR("wrong address family");
          return false;
     }

}

struct igmp_dev mc_tables::get_igmp_table_empty_dev(){
     struct igmp_dev d;
     d.if_name = "??";
     d.index = -1;
     d.g_count = -1;
     d.querier_version = -1;
     return d;
}

struct igmp_dev mc_tables::get_igmp_table_dev(string if_name){
     HC_LOG_TRACE("");

     struct igmp_dev item;
     for(unsigned int i=0; i< m_igmp_table.size();i++){
          item = m_igmp_table[i];
          if(item.if_name.compare(if_name) == 0){
               return item;
          }
     }

     return get_igmp_table_empty_dev();
}

vector<struct igmp_dev> mc_tables::get_igmp_table_all(){
     HC_LOG_TRACE("");

     return m_igmp_table;
}

vector<string> mc_tables::get_igmp_table_all_interfaces(){
     HC_LOG_TRACE("");

     vector<string> v;
     for(unsigned int i=0;i < m_igmp_table.size();i++){
          v.push_back(m_igmp_table[i].if_name);
     }

     return v;
}
