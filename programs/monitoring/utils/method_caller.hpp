#ifndef METHOD_CALLER_HPP
#define METHOD_CALLER_HPP
#include <map>
#include <string>
#include <vector>

class method_caller;

typedef std::string (method_caller::*function_call)(const std::vector<std::string>&);

typedef std::map<std::string, function_call> call_map;

class method_caller
{

private:
    call_map m_calls;
    std::string m_daemon_id;

public:
    inline const std::string& get_daemon_id(){
        return m_daemon_id;
    }

    method_caller()
    {}

    /**
      * @brief creates a method caller object
      * @param Daemon id
      */

    method_caller(std::string daemon_id) : m_daemon_id (daemon_id)
    {}

    /**
      * @brief Call to register a function
      * @param name = the function name
      * @param call = function pointer
      */

    inline void register_function(std::string name,function_call call)
    {
        m_calls[name]= call;
    }

    /**
      * @brief invokes a registered function
      * @param name = function name
      * @param args = arguments for the funtion
      */

    std::string invoke_function(std::string name, std::vector<std::string> args)
    {
        call_map::const_iterator iter = m_calls.find(name);
        if (iter == m_calls.end())
        {
            // call not found return empty string
            return std::string("");
        }
        return ((*this).*(iter->second))(args);
    }

};

#endif // METHOD_CALLER_HPP
