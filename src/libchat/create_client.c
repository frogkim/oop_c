// external_create.c
// 
#include "libchat_client.h"
#ifdef LINUX
#else

p_Client_t __stdcall CreateClient_or_null(uint32_t buffer_size)
{
    p_Client_original_t ret = malloc(sizeof(Client_original_t));
    if (ret == NULL) {
        return NULL;
    }
    memset(ret, 0, sizeof(Client_original_t));
    // public
    ret->setup = _setup_client;
    ret->init = _init_client;
    ret->deinit = _deinit_client;
    ret->connect = _connect_client;
    ret->disconnect = _disconnect_client;
    ret->resume = _resume_client;
    ret->stop = _stop_client;
    ret->recv_async = _recv_async_client;
    ret->send_async = _send_async_client;

    // private
    ret->size_buffer = buffer_size;
    ret->_terminate = FALSE;
    ret->_isconnect = FALSE;
    ret->_isstop = FALSE;
    GetSystemInfo(&ret->_info);

    ret->_send_keyholder = 0;
    ret->_capacity_recvs = 10000;
    ret->_capacity_sends = 10000;
    ret->_evt_send = CreateEvent(NULL, TRUE, FALSE, NULL);
    ret->_func_recv = _func_recv_client;
    ret->_func_send = _func_send_client;
    return (p_Client_t)ret;
}



#endif
