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

extern void spin_lock(KEYHOLDER * keyholder);
extern void spin_unlock(KEYHOLDER * keyholder);

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
    LPWSAOVERLAPPED p_wol;
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
    BOOL(*set_tail)(struct _queue_st* self, const void* p_data_in);
    BOOL(*get_front)(struct _queue_st* self, void* p_data_out);
    // buffer
    ALIGN(8) char* _buffer;
} queue_t, * p_queue_t;

extern int32_t init_queue(p_queue_t self, uint32_t capacity, uint32_t data_size);

#endif
