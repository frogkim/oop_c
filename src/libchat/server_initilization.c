// init_deinit_server.c
// 
#include "libchat_internal.h"
#ifdef LINUX
#else

typedef enum _step {
    done,
    malloc_clients,
    malloc_clients_buffer,
    malloc_seats,
    malloc_recvs,
    malloc_sends,
    startup_wsa,
    socket_open,
    socket_bind,
    socket_thread,
    iocp_create,
    iocp_create_threadpool,
    iocp_create_cleanupgroup,
    iocp_create_work,
    recv_create_threadpool,
    recv_create_cleanupgroup,
    recv_create_work,
    send_create_threadpool,
    send_create_cleanupgroup,
    send_create_work,
    all_release

} STEP;

static STEP init_node(p_Server_original_t self);
static STEP init_queues(p_Server_original_t self);
static STEP init_listen(p_Server_original_t self);
static STEP init_iocp(p_Server_original_t self);
static STEP init_recv(p_Server_original_t self);
static STEP init_send(p_Server_original_t self);
static VOID release_server(p_Server_original_t self, STEP step);


BOOL _init_server(p_Server_t _self)
{
    p_Server_original_t self = (p_Server_original_t)_self;
    STEP result;
    result = init_node(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_queues(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_listen(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_iocp(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_recv(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_send(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    return TRUE;
}


BOOL _deinit_server(p_Server_t _self)
{
    p_Server_original_t self = (p_Server_original_t)_self;
    release_server(self, all_release);
    CloseHandle(self->_evt_recv);
    CloseHandle(self->_evt_send);
    WSACleanup();
    safe_release(self);
    return TRUE;
}


VOID release_server(p_Server_original_t self, STEP step)
{
    switch (step) { // intended cascade
    case all_release:
        // no work for send_work
    case send_create_work:
        CloseThreadpoolCleanupGroupMembers(self->_send_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_send_cleanupgroup);
    case send_create_cleanupgroup:
        CloseThreadpool(self->_send_threadpool);
    case send_create_threadpool:
        // no work for recv_work
    case recv_create_work:
        CloseThreadpoolCleanupGroupMembers(self->_recv_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_recv_cleanupgroup);
    case recv_create_cleanupgroup:
        CloseThreadpool(self->_recv_threadpool);
    case recv_create_threadpool:
        // no work for iocp_work
    case iocp_create_work:
        CloseThreadpoolCleanupGroupMembers(self->_iocp_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_iocp_cleanupgroup);
    case iocp_create_cleanupgroup:
        CloseThreadpool(self->_iocp_threadpool);
    case iocp_create_threadpool:
        CloseHandle(self->_h_iocp);
    case iocp_create:
        CloseHandle(self->_h_listen_thread);
    case socket_thread:
        shutdown(self->_s_listen, SD_BOTH);
    case socket_bind:
        closesocket(self->_s_listen);
    case socket_open:
        safe_release((void*)self->_q_send_client);
    case startup_wsa:
    case malloc_sends:
        safe_release((void*)self->_q_recv_client);
    case malloc_recvs:
        safe_release((void*)self->_q_prior_seats_uint32);
    case malloc_seats:
        safe_release(self->_nodes_client_buf);
    case malloc_clients_buffer:
        safe_release(self->_nodes_client);
    case malloc_clients:
        break;
    }
}

STEP init_node(p_Server_original_t self)
{
    size_t size_nodes = sizeof(node_t) * (uint64_t)self->size_client;
    self->_nodes_client = (p_node_t)malloc(size_nodes);
    if (self->_nodes_client == NULL) {
        return malloc_clients;
    }
    memset(self->_nodes_client, 0, size_nodes);

    size_t size_buffer = self->size_buffer * self->size_client;
    self->_nodes_client_buf = (CHAR*)malloc(size_buffer);
    if (self->_nodes_client_buf == NULL) {
        return malloc_clients_buffer;
    }
    memset(self->_nodes_client_buf, 0, size_buffer);


    uint32_t i = 0;
    size_t size = 0;
    while (i < self->size_client) {
        self->_nodes_client[i].wsabuf.buf = self->_nodes_client_buf + size;
        self->_nodes_client[i].wsabuf.len = self->size_buffer;
        i++;
        size += self->_nodes_client[i].wsabuf.len;
    }
    return done;
}

STEP init_queues(p_Server_original_t self)
{
    // queue's memory design
    // ----------------------------------------------------------------
    // | Queue data | buffer                                          |
    // ----------------------------------------------------------------
    // queue[1] means start place of buffer
    // buffer which is a member of queue struct is not used directly.
    // It stores the address of real buffer block.

    uint32_t capacity;
    uint32_t datasize;
    size_t size;

    capacity = self->_capacity_seats;
    datasize = sizeof(uint32_t);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_prior_seats_uint32 = malloc(size);
    if (self->_q_prior_seats_uint32 == NULL) {
        return malloc_seats;
    }
    memset(self->_q_prior_seats_uint32, 0, size);
    self->_q_prior_seats_uint32->_buffer = (PCHAR)&self->_q_prior_seats_uint32[1];
    //*(self->_q_prior_seats_uint32->_buffer) = (PCHAR) & self->_q_prior_seats_uint32[1];
    init_queue(self->_q_prior_seats_uint32, capacity, datasize);

    capacity = self->_capacity_recvs;
    datasize = sizeof(node_t);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_recv_client = malloc(size);
    if (self->_q_recv_client == NULL) {
        return malloc_recvs;
    }
    memset(self->_q_recv_client, 0, size);
    self->_q_recv_client->_buffer = (PCHAR)&self->_q_recv_client[1];
    //*(self->_q_recv_client->_buffer) = (PCHAR) &self->_q_recv_client[1];
    init_queue(self->_q_recv_client, capacity, datasize);

    capacity = self->_capacity_sends;
    datasize = sizeof(node_t);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_send_client = malloc(size);
    if (self->_q_send_client == NULL) {
        return malloc_sends;
    }
    self->_q_send_client->_buffer = (PCHAR)&self->_q_send_client[1];
    //*(self->_q_send_client->_buffer) = (PCHAR)&self->_q_send_client[1];
    init_queue(self->_q_send_client, capacity, datasize);

    return done;
}

STEP init_listen(p_Server_original_t self)
{
    WSADATA wsaData = { 0 };
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return startup_wsa;
    }

    // set listen socket
    self->_s_listen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (self->_s_listen == INVALID_SOCKET) {
#ifdef _DEBUG
#endif
        return socket_open;
    }

    // bind
    int result = 0;
    result = bind(self->_s_listen, (SOCKADDR*)&self->_addrsvr, sizeof(SOCKADDR_IN));
    if (result == SOCKET_ERROR) {
        return socket_bind;
    }

    result = listen(self->_s_listen, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        return socket_bind;
    }

    // run threads
    self->_h_listen_thread = CreateThread(NULL, 0, self->_func_listen, self, 0, NULL);
    if (self->_h_listen_thread == NULL) {
        return socket_thread;
    }

    return done;
}


STEP init_iocp(p_Server_original_t self)
{
    DWORD n_iocp_threads = self->_info.dwNumberOfProcessors / 4;

    // set iocp
    self->_h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, n_iocp_threads);
    if (self->_h_iocp == NULL) {
        return iocp_create;
    }

    self->_iocp_threadpool = CreateThreadpool(NULL);
    if (self->_iocp_threadpool == NULL) {
        return iocp_create_threadpool;
    }

    SetThreadpoolThreadMaximum(self->_iocp_threadpool, n_iocp_threads);
    SetThreadpoolThreadMinimum(self->_iocp_threadpool, 1);

    self->_iocp_cleanupgroup = CreateThreadpoolCleanupGroup();

    TP_CALLBACK_ENVIRON CallBackEnviron;
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    SetThreadpoolCallbackPool(&CallBackEnviron, self->_iocp_threadpool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, self->_iocp_cleanupgroup, NULL);

    if (self->_iocp_cleanupgroup == NULL) {
        return iocp_create_cleanupgroup;
    }

    for (uint32_t i = 0; i < n_iocp_threads; i++) {
        PTP_WORK work = CreateThreadpoolWork(self->_func_iocp, self, &CallBackEnviron);
        if (work == NULL) {
            return iocp_create_work;
        }
        SubmitThreadpoolWork(work);
    }

    //--- Create Timer
    //PTP_TIMER_CALLBACK timercallback = func_callback_timer;
    //PTP_TIMER timer = CreateThreadpoolTimer(timercallback, NULL, &CallBackEnviron);
    //FILETIME FileDueTime;
    //ULARGE_INTEGER ulDueTime;
    //ulDueTime.QuadPart = (ULONGLONG)-(1 * 10 * 1000 * 1000);
    //FileDueTime.dwHighDateTime = ulDueTime.HighPart;
    //FileDueTime.dwLowDateTime = ulDueTime.LowPart;
    //SetThreadpoolTimer(timer, &FileDueTime, 0, 0);

    return done;
}


STEP init_recv(p_Server_original_t self)
{
    DWORD n_recv_threads = self->_info.dwNumberOfProcessors / 4;
    self->_recv_threadpool = CreateThreadpool(NULL);
    if (self->_recv_threadpool == NULL) {
        return recv_create_threadpool;
    }

    SetThreadpoolThreadMaximum(self->_recv_threadpool, n_recv_threads);
    SetThreadpoolThreadMinimum(self->_recv_threadpool, 1);

    self->_recv_cleanupgroup = CreateThreadpoolCleanupGroup();

    TP_CALLBACK_ENVIRON CallBackEnviron;
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    SetThreadpoolCallbackPool(&CallBackEnviron, self->_recv_threadpool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, self->_recv_cleanupgroup, NULL);

    if (self->_recv_cleanupgroup == NULL) {
        return recv_create_cleanupgroup;
    }

    for (uint32_t i = 0; i < n_recv_threads; i++) {
        PTP_WORK work = CreateThreadpoolWork(self->_func_recv, self, &CallBackEnviron);
        if (work == NULL) {
            return recv_create_work;
        }
        SubmitThreadpoolWork(work);
    }
    return done;
}

STEP init_send(p_Server_original_t self)
{
    DWORD n_send_threads = self->_info.dwNumberOfProcessors / 4;
    self->_send_threadpool = CreateThreadpool(NULL);
    if (self->_send_threadpool == NULL) {
        return send_create_threadpool;
    }

    SetThreadpoolThreadMaximum(self->_send_threadpool, n_send_threads);
    SetThreadpoolThreadMinimum(self->_send_threadpool, 1);

    self->_send_cleanupgroup = CreateThreadpoolCleanupGroup();

    TP_CALLBACK_ENVIRON CallBackEnviron;
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    SetThreadpoolCallbackPool(&CallBackEnviron, self->_send_threadpool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, self->_send_cleanupgroup, NULL);

    if (self->_send_cleanupgroup == NULL) {
        return send_create_cleanupgroup;
    }

    for (uint32_t i = 0; i < n_send_threads; i++) {
        PTP_WORK work = CreateThreadpoolWork(self->_func_send, self, &CallBackEnviron);
        if (work == NULL) {
            return send_create_work;
        }
        SubmitThreadpoolWork(work);
    }
    return done;
}

#endif