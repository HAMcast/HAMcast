#ifndef VOID_HPP
#define VOID_HPP

#include <vector>

#include "hamcast/hamcast_module.h"

class void_module;

void void_module_generator_loop(void_module*);

class void_module
{

    friend void void_module_generator_loop(void_module*);

 public:

    typedef hamcast::uri uri;

    void_module(hc_event_callback_t ecb, hc_recv_callback_t rcb);

    ~void_module();

    void set_handle(hc_module_instance_handle_t hdl);

    void join(const uri& what);

    void leave(const uri& what);

    uri map(const uri& what);

    void send(const uri& whom, const void* data, int len, unsigned char ttl);

    void neighbor_set(std::vector<uri>& storage);

    void group_set(std::vector<std::pair<uri, int> >& storage);

    void children_set(std::vector<uri>& storage, const uri& group);

    void parent_set(std::vector<uri>& storage, const uri& group);

    bool designated_host(const uri& group);

 private:

    void generator_loop();

    hc_recv_callback_t m_recv;
    hc_event_callback_t m_event_cb;
    hc_module_instance_handle_t m_handle;
    boost::thread generator_thread;
    boost::mutex generator_mtx;
    bool generator_running;

};

#endif // VOID_HPP
