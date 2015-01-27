#include "monitor.hpp"
#include "tcp_connection.hpp"
#include <QTimer>
#include <QAbstractSocket>
#include <QDebug>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <QDebug>

monitor::monitor(QObject *parent, int &update_rate,QMessageBox& connection_error):
    QObject(parent),m_tcp_connection(this), m_update_rate(update_rate),m_refresh_timer(this),m_connection_error(connection_error)
{
    connect_all_slots();
}

monitor::~monitor(){
    m_tcp_connection.close_socket();
}

void monitor::connect_all_slots(){
    connect(&m_tcp_connection,SIGNAL(connection_error(QAbstractSocket::SocketError)),this,SLOT(handel_connection_error(QAbstractSocket::SocketError)));
}

bool monitor::connect_to_collector(QString& addr, int& port){
    bool connected;
    connected = m_tcp_connection.connect_to_host(addr,port);
    return connected;
}

void monitor::handel_connection_error(QAbstractSocket::SocketError err){
    bool no_window = false;
    switch (err) {
         case QAbstractSocket::RemoteHostClosedError:
              m_connection_error.setText("The connection was closed by the host");
             break;
         case QAbstractSocket::HostNotFoundError:
             m_connection_error.setText("The host was not found. Please check the "
                                        "host name and port settings.");
             break;
         case QAbstractSocket::ConnectionRefusedError:
             m_connection_error.setText("The connection was refused by the peer. "
                                         "Make sure the server is running, "
                                         "and check that the host name and port "
                                         "settings are correct.");
             break;
    case QAbstractSocket::SocketTimeoutError: no_window = true; break;
         default:
             m_connection_error.setText("Unknown error appeard");
         }
    if(no_window){
        return;
    }
    m_connection_error.setWindowTitle("Tcp connection");
    m_connection_error.show();
}

QList<QString> monitor::get_group_list(){
    std::string meth = "/group_list";
    std::vector<std::string> args;
    http_message send_msg(meth,args);
    QString send_data = QString::fromStdString(send_msg.to_string());
    m_tcp_connection.send(send_data);
    QString tcp_data =m_tcp_connection.receive();
    qDebug() << tcp_data;
    std::string parsemsg = tcp_data.toStdString();
    http_message msg(parsemsg);
    std::string payload = msg.get_payload();
    std::stringstream input(payload);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    QList<QString> group_list = group_list_received(pt);
    return group_list;
}

QList<QString> monitor::get_group_data(QString& group){
    std::string meth = "/group_data";
    std::vector<std::string> args;
    std::string local_group = group.toStdString();
    args.push_back(local_group);
    http_message send_msg(meth,args);
    QString data = QString::fromStdString(send_msg.to_string());
    m_group_data.clear();
    m_group_name = local_group;
    m_tcp_connection.send(data);
    std::string tcp_data =m_tcp_connection.receive().toStdString();
    http_message msg(tcp_data);
    std::cout << tcp_data <<std::endl;
    std::string payload = msg.get_payload();
    std::stringstream input(payload);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    QList<QString> group_data = group_data_received(pt);;
    return group_data;
}

hamcast_node monitor::get_node_data(QString& node_name){
    std::string meth = "/node_data";
    std::vector<std::string> args;
    std::string local_name = node_name.toStdString();
    args.push_back(local_name);
    http_message send_msg(meth,args);
    QString data = QString::fromStdString(send_msg.to_string());
    m_tcp_connection.send(data);
    std::string tcp_data =m_tcp_connection.receive().toStdString();
    http_message msg(tcp_data);

    std::string payload = msg.get_payload();  
    std::stringstream input(payload);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
        qDebug() << QString().fromStdString(payload);
        qDebug() << "paload: = " << payload.length();
        qDebug() << QString().fromStdString(tcp_data);
        qDebug() << "tcp_data : ="<< tcp_data.length();
    }
    hamcast_node node_data = node_data_received(pt);
    return node_data;
}

 QList<QPair<QString,QString>  > monitor::get_group_tree(QString & group_name){
    std::string meth = "/group_tree";
    std::vector<std::string> args;
    std::string local_name = group_name.toStdString();
    args.push_back(local_name);
    http_message send_msg(meth,args);
    QString data = QString::fromStdString(send_msg.to_string());
    m_tcp_connection.send(data);
    std::string tcp_data = m_tcp_connection.receive().toStdString();
    http_message msg(tcp_data);
    std::string payload = msg.get_payload();
    std::stringstream input(payload);
    boost::property_tree::ptree pt;
    try{
        boost::property_tree::xml_parser::read_xml(input,pt);
    }
    catch(std::exception& e){
        std::cout << e.what() << std::endl;
    }
    QList<QPair<QString,QString>  > group_tree = group_tree_received(pt);;
    return group_tree;
}

QList<QString> monitor::group_list_received(boost::property_tree::ptree& data){
   QList<QString> local_list;
   try{
   BOOST_FOREACH(boost::property_tree::ptree::value_type &v,data.get_child("groups")){
        local_list.append(QString::fromStdString(v.second.data()));
    }
}
   catch(std::exception& e){

   }
    return local_list;
}

hamcast_node monitor::node_data_received(boost::property_tree::ptree& data){
    hamcast_node n(data);
    return n;
}

QList<QString> monitor::group_data_received(boost::property_tree::ptree &data){
    QList<QString> local_list;
    std::string s = data.get("group.member","");
     BOOST_FOREACH(boost::property_tree::ptree::value_type &v,data.get_child("group")){
         if(v.first == "name"){
             continue;
         }
         else{
            local_list.append(QString::fromStdString(v.second.data()));
        }
     }
     return local_list;
}

QList<QPair<QString,QString>  > monitor::group_tree_received(boost::property_tree::ptree& data){
    QList<QPair<QString,QString>  > edge_list;
    try{
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v,data.get_child("edges")){
        std::string key = v.first.data();
        boost::property_tree::ptree tmp = data.get_child("edges."+key);
        QPair<QString,QString> edge;
        edge.first = QString::fromStdString(tmp.get("from",""));
        edge.second = QString::fromStdString(tmp.get("to",""));
        edge_list.append(edge);
    }
}
    catch(std::exception & e){

    }
    return edge_list;
}
