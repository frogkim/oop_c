#include "libchat_internal.h"

#ifdef LINUX
#else
void _func_recv_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(work);

    p_Server_original_t self = pParam;
    p_queue_t       p_seat_queue = self->_q_prior_seats_uint32;
    p_queue_t       p_recv_queue = self->_q_recv_client;
    p_queue_t       p_send_queue = self->_q_send_client;
    HANDLE          recv_evt = &self->_evt_recv;
    HANDLE          send_evt = &self->_evt_send;
    PKEYHOLDER      p_keyholder_recv = &self->_keyholder_recv;
    PKEYHOLDER      p_keyholder_send = &self->_keyholder_send;

    DWORD dw_event;
    node_t client;

    while (TRUE) {
        dw_event = WaitForSingleObject(recv_evt, INFINITE);
        if (self->_terminate) {
            break;
        }
        while (TRUE) {
            if (p_recv_queue->size > 0) {
                p_recv_queue->get_front(p_recv_queue, &client);
#ifdef DEBUG
                puts("received from client");
#endif // DEBUG
                // working
                p_send_queue->set_tail(p_send_queue, &client);
#ifdef DEBUG
                puts("ready for replying to client");
#endif // DEBUG
                // end working
            } else {
                break;
            }
            spin_lock(p_keyholder_recv);
            ResetEvent(recv_evt);
            spin_unlock(p_keyholder_recv);

            spin_lock(p_keyholder_send);
            SetEvent(send_evt);
            spin_unlock(p_keyholder_send);
        }
    }
}

#endif