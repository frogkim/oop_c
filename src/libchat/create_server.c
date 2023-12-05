// external_create.c
// 
#include "libchat_server.h"
#ifdef LINUX
#else


p_Server_t __stdcall CreateServer_or_null(unsigned int n_clients, unsigned int buffer_size)
{
    p_Server_original_t ret = malloc(sizeof(Server_original_t));
    if (ret == NULL) {
        return NULL;
    }
    memset(ret, 0, sizeof(Server_original_t));
    // public
    ret->init = _init_server;
    ret->deinit = _deinit_server;
    ret->stop = _stop_server;
    ret->resume = _resume_server;

    // private
    memset(&ret->_addrsvr, 0, sizeof(SOCKADDR_IN));
    ret->_addrsvr.sin_family = AF_INET;
    ret->_addrsvr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    ret->_addrsvr.sin_port = htons(25000);
    GetSystemInfo(&ret->_info);
    ret->_terminate = 0;
    ret->size_client = n_clients;
    ret->size_buffer = buffer_size;
    ret->_stop = 0;
    ret->_capacity_seats = 5000;
    ret->_capacity_works = 10000;
    ret->_capacity_sends = 10000;
    
    ret->_func_listen=_func_listen_server;
    ret->_func_iocp=_func_iocp_server;
    ret->_func_work=_func_work_server;
    ret->_func_send=_func_send_server;
    return (p_Server_t)ret;
}

#endif
