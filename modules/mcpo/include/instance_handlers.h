#ifndef INSTANCE_HANDLERS_H
#define INSTANCE_HANDLERS_H
#include "hamcast/hamcast_module.h"
struct InstanceHandlers{
    hc_module_instance_handle_t m_handle;
    hc_recv_callback_t recv_cb;
    hc_event_callback_t event_cb;
    hc_log_fun_t log_fun;
    hc_kvp_list_t* kvp;
};




#endif // INSTANCE_HANDLERS_H
