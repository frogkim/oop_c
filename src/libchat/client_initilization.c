// init_deinit_server.c
// 
#include "libchat_client.h"
#ifdef LINUX
#else
typedef enum _step {
    all_release,
    malloc_buffer,
    malloc_q_recvs,
    malloc_q_sends,
    done,
} STEP;

static VOID release_client(p_Client_original_t self, STEP step);
static STEP init_mem(p_Client_original_t self);
static STEP init_queues(p_Client_original_t self);

BOOL _init_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    STEP result;
    result = init_mem(self);
    if (result != done) {
        release_client(self, result);
        return FALSE;
    }
    result = init_queues(self);
    if (result != done) {
        release_client(self, result);
        return FALSE;
    }
    return TRUE;
}

BOOL _deinit_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    release_client(self, all_release);
    CloseHandle(self->_evt_send);
    safe_release(self);
    return TRUE;
}

VOID release_client(p_Client_original_t self, STEP step)
{
    switch (step) { // intended cascade
    case all_release:
        safe_release(self->_q_send);
    case malloc_q_sends:
        safe_release(self->_q_recv);
    case malloc_q_recvs:
        //safe_release(self->_buffer);
    case malloc_buffer:
        break;
    }
}

STEP init_mem(p_Client_original_t self)
{
    //self->_buffer = malloc(NODE_BUFFER_SIZE);
    //if (self->_buffer == NULL) {
    //    return malloc_buffer;
    //}
    //memset(self->_buffer, 0, NODE_BUFFER_SIZE);
    return done;
}

STEP init_queues(p_Client_original_t self)
{
    uint32_t capacity;
    uint32_t datasize;
    size_t size;

    capacity = self->_capacity_recvs;
    datasize = sizeof(node_t);
    size = sizeof(queue_t) + datasize * capacity;


    self->_q_recv = malloc(size);
    if (self->_q_recv == NULL) {
        return malloc_q_recvs;
    }
    memset(self->_q_recv, 0, size);
    self->_q_recv->_buffer = (PCHAR)&self->_q_recv[1];
    init_queue(self->_q_recv, capacity, datasize);

    self->_q_send = (p_queue_t)malloc(size);
    if (self->_q_send == NULL) {
        return malloc_q_sends;
    }
    memset(self->_q_send, 0, size);
    self->_q_send->_buffer = (PCHAR)&self->_q_send[1];
    init_queue(self->_q_send, capacity, datasize);
    return done;
}

#endif