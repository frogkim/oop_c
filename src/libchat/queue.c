#include "libchat_common.h"

#ifdef LINUX
#else
#endif

BOOL  _set_tail(p_queue_t, const void* in);
BOOL  _get_front(p_queue_t, void* out);

BOOL init_queue(p_queue_t self, uint32_t capacity, uint32_t data_size)
{
    if (capacity == 0 || data_size == 0) {
        return FALSE;
    }

    self->front = 0;
    self->tail = 0;
    self->size = 0;
    self->capacity = capacity;
    self->data_size = data_size;
    self->set_tail = _set_tail;
    self->get_front = _get_front;
#ifdef LINUX
#else
    InitializeCriticalSection(&self->_cs);
#endif
    return TRUE;
}

void deinit_queue(p_queue_t self)
{
    if(self == NULL) {
        return;
    }

    if (self->_buffer != NULL) {
        free(self->_buffer);
    }

#ifdef LINUX

#else
    DeleteCriticalSection(&self->_cs);
#endif

}

BOOL _set_tail(p_queue_t self, const void* data_in)
{
    if (self->size == self->capacity) {
        return FALSE;
    }


#ifdef LINUX

#else
    EnterCriticalSection(&self->_cs);
#endif
    memcpy(self->_buffer + (self->data_size * self->tail), data_in, self->data_size);
    self->tail++;
    self->size++;

    if (self->tail == self->capacity) {
        self->tail = 0;
    }

#ifdef LINUX

#else
    LeaveCriticalSection(&self->_cs);
#endif
    return TRUE;
}

BOOL _get_front(p_queue_t self, void* data_out)
{
    if (self->size == 0) {
        return FALSE;
    }

#ifdef LINUX

#else
    EnterCriticalSection(&self->_cs);
#endif
    memcpy(data_out, self->_buffer + (self->data_size * self->front), self->data_size);
    self->front++;
    self->size--;
    if (self->front == self->capacity) {
        self->front = 0;
    }
#ifdef LINUX

#else
    LeaveCriticalSection(&self->_cs);
#endif
    return TRUE;
}
