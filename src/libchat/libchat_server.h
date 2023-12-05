#include "libchat_common.h"
typedef struct _work_parameter {
    p_queue_t q_work;
    p_queue_t q_send;
    p_queue_t q_work_events;
    HANDLE evt;
    BOOL* terminate;
    BOOL* stop;
}work_parameter, *p_work_parameter;

typedef struct _send_parameter {
    p_queue_t q_send;
    p_queue_t q_send_events;
    HANDLE evt;
    BOOL* terminate;
    BOOL* stop;
}send_parameter, * p_send_parameter;

typedef struct _server_original {

    // public
    BOOL(*init)(p_Server_t);
    BOOL(*deinit)(p_Server_t);
    VOID(*stop)(p_Server_t);
    VOID(*resume)(p_Server_t);

    // private has _ prefix
    uint32_t size_client;
    uint32_t size_buffer;
    SOCKADDR_IN _addrsvr;
    SYSTEM_INFO _info;
    uint32_t _terminate;
    uint32_t _stop;
    uint32_t _capacity_seats;
    uint32_t _capacity_works;
    uint32_t _capacity_sends;
    uint32_t _reserved;

    DWORD(*_func_listen)(LPVOID);
    PTP_WORK_CALLBACK _func_iocp;
    PTP_WORK_CALLBACK _func_work;
    PTP_WORK_CALLBACK _func_send;

    p_node_t _nodes_client;
    CHAR* _nodes_client_buf;

    p_queue_t _q_prior_seats_uint32;
    p_queue_t _q_work;
    p_queue_t _q_send;
    p_queue_t _q_work_events;
    p_queue_t _q_send_events;
    PHANDLE _work_events;
    PHANDLE _send_events;
    DWORD _n_work_threads;
    DWORD _n_send_threads;
    // handle and socket
    SOCKET _s_listen;
    HANDLE _h_listen_thread;

    HANDLE _h_iocp;
    PTP_POOL _iocp_threadpool;
    PTP_CLEANUP_GROUP _iocp_cleanupgroup;

    PTP_POOL _work_threadpool;
    PTP_CLEANUP_GROUP _work_cleanupgroup;

    PTP_POOL _send_threadpool;
    PTP_CLEANUP_GROUP _send_cleanupgroup;
} Server_original_t, * p_Server_original_t;

// initialize
extern BOOL _init_server(p_Server_t);
extern void _stop_server(p_Server_t);
extern void _resume_server(p_Server_t);
extern BOOL _deinit_server(p_Server_t);

// thread callback functions
extern DWORD CALLBACK _func_listen_server(LPVOID);
extern void _func_iocp_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work);
extern void _func_work_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work);
extern void _func_send_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work);
