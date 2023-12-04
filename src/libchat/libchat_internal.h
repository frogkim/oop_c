#include "libchat.h"
#include "stdint.h"
#include "stdlib.h"
#include "time.h"

#ifdef DEBUG
#include <assert.h>
#include <stdio.h>
#endif

// common
// Export type
#define safe_release(p) if(p) free(p)
#define ALIGN(x) __declspec(align(x))
typedef volatile unsigned long long KEYHOLDER, * PKEYHOLDER;


#ifdef LINUX
#else
#include "WinSock2.h"
#pragma comment(lib, "ws2_32")
#include "ws2tcpip.h"
#include "windows.h"

typedef struct _node_st {
    uint32_t is_stop;
    uint32_t index;
    SOCKET  socket;
    WSABUF  wsabuf;
    uint32_t n_recv;
    uint32_t flag;
    WSAOVERLAPPED wol;
    struct timespec ts;                     // timespec_get(&ts, TIME_UTC);
    LPWSAOVERLAPPED_COMPLETION_ROUTINE(*_func_wsarecv_callback)(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
    LPWSAOVERLAPPED_COMPLETION_ROUTINE(*_func_wsasend_callback)(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
} node_t, * p_node_t;

typedef struct _queue_st {
    uint32_t front;
    uint32_t tail;
    uint32_t size;
    uint32_t _capacity;
    uint32_t _data_size;
    CRITICAL_SECTION _cs;
    BOOL (*set_tail)(struct _queue_st* self, const void* p_data_in);
    BOOL (*get_front)(struct _queue_st* self, void* p_data_out);
    // buffer
    ALIGN(8) char* _buffer;
} queue_t, * p_queue_t;

typedef struct _server_original {

    // public
    BOOL (*init)(p_Server_t);
    BOOL (*deinit)(p_Server_t);
    VOID (*stop)(p_Server_t);
    VOID (*resume)(p_Server_t);

    // private has _ prefix
    uint32_t size_client;
    uint32_t size_buffer;
    SOCKADDR_IN _addrsvr;
    SYSTEM_INFO _info;
    uint64_t _terminate;
    KEYHOLDER _keyholder_seat;
    KEYHOLDER _keyholder_recv;
    KEYHOLDER _keyholder_send;
    uint32_t _stop;
    uint32_t _capacity_seats;
    uint32_t _capacity_recvs;
    uint32_t _capacity_sends;
    HANDLE _evt_recv;
    HANDLE _evt_send;

    DWORD (*_func_listen)(LPVOID);
    PTP_WORK_CALLBACK _func_iocp;
    PTP_WORK_CALLBACK _func_recv;
    PTP_WORK_CALLBACK _func_send;

    p_node_t _nodes_client;
    CHAR* _nodes_client_buf;

    p_queue_t _q_prior_seats_uint32;
    p_queue_t _q_recv_client;
    p_queue_t _q_send_client;

    // handle and socket
    SOCKET _s_listen;
    HANDLE _h_listen_thread;

    HANDLE _h_iocp;
    PTP_POOL _iocp_threadpool;
    PTP_CLEANUP_GROUP _iocp_cleanupgroup;

    PTP_POOL _recv_threadpool;
    PTP_CLEANUP_GROUP _recv_cleanupgroup;

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
extern void _func_recv_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work);
extern void _func_send_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work);

typedef struct _client_original {
    // public
    BOOL (*setup)(p_Client_t, char* ip_or_null, unsigned short port);
    BOOL (*init)(p_Client_t);
    BOOL (*deinit)(p_Client_t);
    BOOL (*connect)(p_Client_t);
    BOOL (*disconnect)(p_Client_t);
    VOID (*resume)(p_Client_t);
    VOID (*stop)(p_Client_t);
    VOID (*recv_async)(p_Client_t, void* data_out);
    VOID (*send_async)(p_Client_t, void* data_in);

    // private has _ prefix
    uint32_t size_buffer;
    uint32_t _terminate;
    uint32_t _isconnect;
    uint32_t _isstop;
    SYSTEM_INFO _info;
    SOCKADDR_IN _addrsvr_in;

    // handle and socket
    SOCKET _socket;
    HANDLE _socket_thread;
    CHAR*  _buffer;                 // release

    HANDLE _h_recv_thread;
    HANDLE _h_send_thread;

    p_queue_t _q_recv;
    p_queue_t _q_send;

    KEYHOLDER _keyholder_recv;
    KEYHOLDER _keyholder_send;
    uint32_t _capacity_recvs;
    uint32_t _capacity_sends;
    HANDLE _evt_recv;
    HANDLE _evt_send;

    DWORD(*_func_recv)(LPVOID);
    DWORD(*_func_send)(LPVOID);
} Client_original_t, * p_Client_original_t;

// initialize
extern BOOL _setup_client(p_Client_t, char* ip, unsigned short port);
extern BOOL _init_client(p_Client_t);
extern BOOL _connect_client(p_Client_t);
extern BOOL _disconnect_client(p_Client_t);
extern VOID _resume_client(p_Client_t);
extern VOID _stop_client(p_Client_t);
extern VOID _recv_async_client(p_Client_t, void* data_out);
extern VOID _send_async_client(p_Client_t, void* data_in);
extern BOOL _deinit_client(p_Client_t);


// thread callback functions
extern DWORD CALLBACK _func_recv_client(LPVOID);
extern DWORD CALLBACK _func_send_client(LPVOID);
#endif

// Common
extern void spin_lock(KEYHOLDER* keyholder);
extern void spin_unlock(KEYHOLDER* keyholder);
extern int32_t init_queue(p_queue_t self, uint32_t capacity, uint32_t data_size);

