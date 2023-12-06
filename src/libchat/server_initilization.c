// init_deinit_server.c
// 
#include "libchat_server.h"
#ifdef LINUX
#else
#pragma warning(disable : 4996)

typedef enum _step {
    done,
    malloc_clients,
    malloc_seats,
    malloc_works,
    malloc_works_events,
    malloc_sends,
    malloc_sends_events,
    startup_wsa,
    socket_open,
    socket_bind,
    socket_thread,
    iocp_create,
    iocp_create_threadpool,
    iocp_create_cleanupgroup,
    iocp_create_work,
    work_create_threadpool,
    work_create_cleanupgroup,
    work_create_work,
    send_create_threadpool,
    send_create_cleanupgroup,
    send_create_work,
    all_release

} STEP;

static STEP init_node(p_Server_original_t self);
static STEP init_queues(p_Server_original_t self);
static STEP init_listen(p_Server_original_t self);
static STEP init_iocp(p_Server_original_t self);
static STEP init_work(p_Server_original_t self);
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

    result = init_work(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_send(self);
    if (result != done) {
        release_server(self, result);
        return FALSE;
    }

    result = init_iocp(self);
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
    WSACleanup();
    safe_release(self);
    return TRUE;
}


VOID release_server(p_Server_original_t self, STEP step)
{
    switch (step) { // intended cascade
    case all_release:
        // no work for send_work
    case iocp_create_work:
        CloseThreadpoolCleanupGroupMembers(self->_iocp_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_iocp_cleanupgroup);
    case iocp_create_cleanupgroup:
        CloseThreadpool(self->_iocp_threadpool);
    case iocp_create_threadpool:
        safe_release(self->send_parameters);
    case send_create_work:
        CloseThreadpoolCleanupGroupMembers(self->_send_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_send_cleanupgroup);
    case send_create_cleanupgroup:
        CloseThreadpool(self->_send_threadpool);
    case send_create_threadpool:
        safe_release(self->work_parameters);
    case work_create_work:
        safe_release(self->_work_events);
        CloseThreadpoolCleanupGroupMembers(self->_work_cleanupgroup, FALSE, NULL);
        CloseThreadpoolCleanupGroup(self->_work_cleanupgroup);
    case work_create_cleanupgroup:
        CloseThreadpool(self->_work_threadpool);
    case work_create_threadpool:

        CloseHandle(self->_h_iocp);
    case iocp_create:
        CloseHandle(self->_h_listen_thread);
    case socket_thread:
        shutdown(self->_s_listen, SD_BOTH);
    case socket_bind:
        closesocket(self->_s_listen);
    case socket_open:
    case startup_wsa:
        safe_release(self->_q_send_events);
    case malloc_sends_events:
        safe_release(self->_q_send);
    case malloc_sends:
        safe_release(self->_q_work_events);
    case malloc_works_events:
        safe_release(self->_q_work);
    case malloc_works:
        safe_release(self->_q_prior_seats_uint32);
    case malloc_seats:
        safe_release(self->_send_events);
        safe_release(self->_work_events);
        safe_release(self->_nodes_client);
    case malloc_clients:
        break;
    }
}

STEP init_node(p_Server_original_t self)
{
    size_t size = sizeof(node_t) * self->size_client;
    self->_nodes_client = (p_node_t)malloc(size);
    if (self->_nodes_client == NULL) {
        return malloc_clients;
    }
    memset(self->_nodes_client, 0, size);
    for(uint32_t i = 0; i < self->size_client; i++) {
        self->_nodes_client[i].wsabuf.buf = self->_nodes_client[i]._buffer;
        self->_nodes_client[i].wsabuf.len = NODE_BUFFER_SIZE;
        self->_nodes_client[i].p_wol = &self->_nodes_client[i].wol;
    }

    DWORD n_iocp_threads = self->_info.dwNumberOfProcessors / 4;
    WCHAR event_name[20] = { 0 };
    self->_n_send_threads = n_iocp_threads;
    self->_n_work_threads = n_iocp_threads;
    self->_work_events = (PHANDLE)malloc(sizeof(HANDLE) * self->_n_send_threads);
    self->_send_events = (PHANDLE)malloc(sizeof(HANDLE) * self->_n_work_threads);
    
    memset(event_name, 0, sizeof(WCHAR) * 20);
    wcscpy(event_name, TEXT("work_"));
    for (uint32_t i = 0; i < self->_n_work_threads; i++) {
        event_name[5] = i + '0';
        self->_work_events[i] = CreateEvent(NULL, FALSE, FALSE, event_name);
    }

    memset(event_name, 0, sizeof(WCHAR) * 20);
    wcscpy(event_name, TEXT("send_"));
    for (uint32_t i = 0; i < self->_n_send_threads; i++) {
        event_name[5] = i + '0';
        self->_send_events[i] = CreateEvent(NULL, FALSE, FALSE, event_name);
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

    // seats queue
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

    // works queue
    capacity = self->_capacity_works;
    datasize = sizeof(node_t);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_work = malloc(size);
    if (self->_q_work == NULL) {
        return malloc_works;
    }
    memset(self->_q_work, 0, size);
    self->_q_work->_buffer = (PCHAR)&self->_q_work[1];
    init_queue(self->_q_work, capacity, datasize);

    // works events queue
    capacity = self->_capacity_works;
    datasize = sizeof(HANDLE);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_work_events = malloc(size);
    if (self->_q_work_events == NULL) {
        return malloc_works_events;
    }
    memset(self->_q_work_events, 0, size);
    self->_q_work_events->_buffer = (PCHAR)&self->_q_work_events[1];
    init_queue(self->_q_work_events, capacity, datasize);

    // sends queue
    capacity = self->_capacity_sends;
    datasize = sizeof(node_t);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_send = malloc(size);
    if (self->_q_send == NULL) {
        return malloc_sends;
    }
    memset(self->_q_send, 0, size);
    self->_q_send->_buffer = (PCHAR)&self->_q_send[1];
    init_queue(self->_q_send, capacity, datasize);

    // sends events queue
    capacity = self->_capacity_works;
    datasize = sizeof(HANDLE);
    size = sizeof(queue_t) + datasize * capacity;
    self->_q_send_events = malloc(size);
    if (self->_q_send_events == NULL) {
        return malloc_works_events;
    }
    memset(self->_q_send_events, 0, size);
    self->_q_send_events->_buffer = (PCHAR)&self->_q_send_events[1];
    init_queue(self->_q_send_events, capacity, datasize);

    
    for (uint32_t i = 0; i < self->_n_work_threads; i++) {
        self->_q_work_events->set_tail(self->_q_work_events, &self->_work_events[i]);
    }
    self->work_parameters = malloc(sizeof(work_parameter_t) * self->_n_work_threads);
    memset(self->work_parameters, 0, sizeof(work_parameter_t) * self->_n_work_threads);

    for (uint32_t i = 0; i < self->_n_send_threads; i++) {
        self->_q_send_events->set_tail(self->_q_send_events, &self->_send_events[i]);
    }
    self->send_parameters = malloc(sizeof(send_parameter_t) * self->_n_send_threads);
    memset(self->send_parameters, 0, sizeof(send_parameter_t) * self->_n_work_threads);
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


STEP init_work(p_Server_original_t self)
{
    self->_n_work_threads = self->_info.dwNumberOfProcessors / 4;
    self->_work_threadpool = CreateThreadpool(NULL);
    if (self->_work_threadpool == NULL) {
        return work_create_threadpool;
    }

    SetThreadpoolThreadMaximum(self->_work_threadpool, self->_n_work_threads);
    SetThreadpoolThreadMinimum(self->_work_threadpool, 1);

    self->_work_cleanupgroup = CreateThreadpoolCleanupGroup();

    TP_CALLBACK_ENVIRON CallBackEnviron;
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    SetThreadpoolCallbackPool(&CallBackEnviron, self->_work_threadpool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, self->_work_cleanupgroup, NULL);

    if (self->_work_cleanupgroup == NULL) {
        return work_create_cleanupgroup;
    }

    for (uint32_t i = 0; i < self->_n_work_threads; i++) {
        self->work_parameters->q_work = self->_q_work;
        self->work_parameters->q_send = self->_q_send;
        self->work_parameters->q_work_events = self->_q_work_events;
        self->work_parameters->q_send_events = self->_q_send_events;
        self->work_parameters->evt = self->_work_events[i];
        self->work_parameters->terminate = &self->_terminate;
        self->work_parameters->stop = &self->_stop;

        PTP_WORK work = CreateThreadpoolWork(self->_func_work, self->work_parameters, &CallBackEnviron);
        if (work == NULL) {
            for (; i > 0; i--) {
                CloseHandle(self->_work_events[i-1]);
            }
            return work_create_work;
        }
        SubmitThreadpoolWork(work);
    }

    return done;
}

STEP init_send(p_Server_original_t self)
{
    self->_n_send_threads = self->_info.dwNumberOfProcessors / 4;
    self->_send_threadpool = CreateThreadpool(NULL);
    if (self->_send_threadpool == NULL) {
        return send_create_threadpool;
    }

    SetThreadpoolThreadMaximum(self->_send_threadpool, self->_n_send_threads);
    SetThreadpoolThreadMinimum(self->_send_threadpool, 1);

    self->_send_cleanupgroup = CreateThreadpoolCleanupGroup();

    TP_CALLBACK_ENVIRON CallBackEnviron;
    InitializeThreadpoolEnvironment(&CallBackEnviron);

    SetThreadpoolCallbackPool(&CallBackEnviron, self->_send_threadpool);
    SetThreadpoolCallbackCleanupGroup(&CallBackEnviron, self->_send_cleanupgroup, NULL);

    if (self->_send_cleanupgroup == NULL) {
        return send_create_cleanupgroup;
    }

    for (uint32_t i = 0; i < self->_n_send_threads; i++) {
        self->send_parameters->q_send = self->_q_send;
        self->send_parameters->q_send_events = self->_q_send_events;
        self->send_parameters->evt = self->_work_events[i];
        self->send_parameters->terminate = &self->_terminate;
        self->send_parameters->stop = &self->_stop; 
        PTP_WORK work = CreateThreadpoolWork(self->_func_send, self, &CallBackEnviron);
        if (work == NULL) {
            for (; i > 0; i--) {
                CloseHandle(self->_send_events[i - 1]);
            }
            return work_create_work;
        }
        SubmitThreadpoolWork(work);
    }

    return done;
}

#endif