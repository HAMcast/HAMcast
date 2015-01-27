#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "http_message.hpp"

using std::string;
using std::vector;

http_message::http_message(const string& msg){
    std::pair<string,string> head_pay;
    head_pay = split_header_payload(msg);
    m_header = head_pay.first;
    m_payload = head_pay.second;
    parse_header(m_header);
    if(m_type == c_post){
        parse_payload(m_payload);
    }
    m_header_size = m_header.length();

}

/**
  * @brief Create POST message
  * @param method Method to be called
  * @param args Arguments for method
  */
http_message::http_message(const string& method, const vector<string> args){
    string header;
    string payload;
    payload = create_payload(args);
    header = create_header("POST", method, payload.size());
    m_header = header;
    m_header_size = header.length();
    m_payload = payload;
    m_type = "POST";
    m_method = method;
    m_args = args;
}

/**
  * @brief Create http reply
  * @param error HTTP Error code for message, eg 200 or 404
  * @param reply_payload [optional]
  */
http_message::http_message(const string& error, string& reply_payload){
    string header;
    header = create_header(error, "", reply_payload.size());
    m_header = header;
     m_header_size = header.length();
    m_payload = reply_payload;
    m_length = reply_payload.size();
    m_type = error;
}


string http_message::create_header (const string& m_type, const string& method, const int& length){
    string header;
    try{
        if(m_type == c_post){
            header = create_post_header(method,length);
        }
        if(m_type == c_200){
            header = create_reply_header(length);
        }
        if(m_type == c_404){
            header = create_404_header(length);
        }
    }
    catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }

    return header;
}

string http_message::create_post_header(const string& method, const int& lenght){
    string header;
    header += c_post + " " + method + " " + c_http_version + c_newline;
    header += c_user_agent + c_newline;
    header += c_content_lenght + boost::lexical_cast<string>(lenght)
                + c_newline;
    header += c_content_plain + c_newline;

    return header;
}

//TODO:: Varriabler
string http_message::create_reply_header(const int& lenght){
    string header;
    header += c_reply_ok+c_newline;
    header += c_content_lenght 
                + " "
                + boost::lexical_cast<string>(lenght)
                + c_newline;
    header += c_content_xml + c_newline;

    return header;
}

string http_message::create_404_header(const int& lenght){
    string header;
    header += c_reply_404+c_newline;
    header += c_content_lenght 
                + " " 
                + boost::lexical_cast<string>(lenght)
                + c_newline;
    header += c_content_xml + c_newline;

    return header;
}

void http_message::parse_header (const string& header){
    std::istringstream arg_stream(header);
    string line;
    std::getline(arg_stream, line);
    size_t post_pos = line.find("POST",0);
    size_t ok_pos = line.find("OK",0);
    size_t pos_404 = line.find("404",0);
    if (post_pos != std::string::npos) {
        size_t linesize = line.size();
        m_method = line.substr(post_pos+5, linesize-15);
        m_type   = line.substr(post_pos,post_pos+4);
        while (std::getline(arg_stream, line)) {
            size_t cont_pos = line.find ("Content-Length", 0);
            if (cont_pos != std::string::npos) {
                string stmp = line.substr (cont_pos+15,line.size()-1);
                std::istringstream buffer(stmp);
                buffer >> m_length;
                break;
            }
        }
    }
    if (ok_pos != std::string::npos) {
        m_type = line.substr(ok_pos, 2);
        while(std::getline(arg_stream, line)){
            size_t cont_pos = line.find ("Content-Length", 0);
            if (cont_pos != std::string::npos) {
                string stmp = line.substr (cont_pos+15,line.size()-1);
                std::istringstream buffer(stmp);
                buffer >> m_length;
                break;
            }
        }
    }

    if (pos_404 != string::npos) {
        m_type = line.substr(pos_404, line.size()-10);
    }
}

string http_message::create_payload (vector<string> args){
    string payload;
    try{
        for(unsigned int i =0; i < args.size();i++){
            payload += "arg"+boost::lexical_cast<std::string>(i)+"="+args[i]+c_newline;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return payload;
}

std::pair<string, string>  http_message::split_header_payload(const string& msg){
    std::istringstream msg_stream(msg);
    string line;
    std::pair<string, string> result;
    size_t splitsize =0;
    bool pay = false;
    bool nonewline;
    while(std::getline(msg_stream, line)){

        if (line.size() < 2) {
            pay = true;
            continue;
        }
        else {
            if (!pay) {
                if(line.find("200") != std::string::npos){
                    nonewline = true;
                }
                result.first += line.substr(0,line.size()-1)+c_newline;
            }
            else {
                if (nonewline) {
                    result.second += line.substr(0,line.size());
                }
                else{
                    result.second += line.substr(0,line.size()-1)+c_newline;
                }
            }
        }
    }
    return result;
}

void http_message::parse_payload (const string& payload){
    std::istringstream arg_stream(payload);
    string line;
    vector<string> result;
    while (std::getline(arg_stream, line)) {
        size_t delim = line.find("=",0);
        size_t length = line.size();
        size_t subleght = delim+2;
        string value = line.substr(delim + 1, length - subleght);
        if (!value.empty())
            result.push_back(value);
    }
    m_args = result;
}
