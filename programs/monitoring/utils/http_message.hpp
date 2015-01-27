#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <vector>

const std::string c_http_version    = "HTTP/1.1";
const std::string c_user_agent      = "User-Agent: HAMcast Monitoring";
const std::string c_content_plain   = "Content-Type: text/plain; charset=utf-8";
const std::string c_reply_ok        = "HTTP/1.1 200 OK";
const std::string c_reply_404       = "HTTP/1.1 404 Not Found";
const std::string c_newline         = "\r\n";
const std::string c_content_xml     = "Content-Type: text/xml; charset=utf-8";
const std::string c_content_lenght  = "Content-Length:";

const std::string c_post            = "POST";
const std::string c_200             = "200";
const std::string c_404             = "404";

class http_message
{
private:

    std::string                 m_header;
    std::string                 m_payload;
    std::string                 m_type;
    std::string                 m_method;
    std::vector<std::string>    m_args;
    size_t                      m_length;
    size_t                      m_header_size;

    std::string create_header (const std::string& m_type, const std::string& method, const int& length);

    void parse_header (const std::string& header);

    std::string create_payload (std::vector<std::string> args);

    void parse_payload (const std::string& payload);

    std::string create_post_header(const std::string& method,const int& lenght);
    std::string create_reply_header(const int& lenght);
    std::string create_404_header(const int& lenght);

    std::pair<std::string,std::string> split_header_payload(const std::string& msg);

public:
    /**
      * @brief Parse http message from string
      * @param msg HTTP message as string
      */
    http_message(const std::string& msg);

    /**
      * @brief Create POST message
      * @param method Method to be called
      * @param args Arguments for method
      */
    http_message(const std::string& method, const std::vector<std::string> args);

    /**
      * @brief Create http reply
      * @param error HTTP Error code for message, eg 200 or 404
      * @param reply_payload [optional]
      */
    http_message(const std::string& error, std::string& reply_payload);

    inline std::string to_string()
    {
        return (m_header + "\r\n" + m_payload);
    }

    inline std::string get_method ()
    {
        return m_method;
    }

    inline std::vector<std::string> get_arguments ()
    {
        return m_args;
    }

    inline std::string get_type ()
    {
        return m_type;
    }

    inline std::string get_payload(){
        return m_payload;
    }

    inline const size_t get_payload_size(){
        return m_length;
    }
    inline const size_t get_header_size(){
        return m_header_size;
    }
};

#endif // HTTP_MESSAGE_H
