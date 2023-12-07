#include "libchat_common.h"

typedef struct _client_original {
    // public
    BOOL(*setup)(p_Client_t, char* ip_or_null, unsigned short port);
    BOOL(*init)(p_Client_t);
    BOOL(*deinit)(p_Client_t);
    BOOL(*connect)(p_Client_t);
    BOOL(*disconnect)(p_Client_t);
    VOID(*resume)(p_Client_t);
    VOID(*stop)(p_Client_t);
    VOID(*recv_async)(p_Client_t, void* data_out);
    VOID(*send_async)(p_Client_t, void* data_in);

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

    HANDLE _h_recv_thread;
    HANDLE _h_send_thread;

    p_queue_t _q_recv;
    p_queue_t _q_send;

    uint32_t _capacity_recvs;
    uint32_t _capacity_sends;
    
    HANDLE _evt_send;
    KEYHOLDER _send_keyholder;

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
