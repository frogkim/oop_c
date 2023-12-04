#include "libchat_internal.h"

#ifdef LINUX
#else
void _func_send_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(pParam);


    p_Server_original_t self = pParam;
    node_t          client;
    p_node_t        p_client = &client;
    p_queue_t       p_send_queue = self->_q_send_client;
    HANDLE          send_evt = &self->_evt_send;
    PKEYHOLDER      p_keyholder_send = &self->_keyholder_send;

    DWORD dw_event;
    DWORD dw_size_sent;
    while (TRUE) {
        dw_event = WaitForSingleObject(send_evt, INFINITE);
        if (self->_terminate) {
            break;
        }
        while (TRUE) {
            if (p_send_queue->size > 0) {
                p_send_queue->get_front(p_send_queue, p_client);
                if (p_client == NULL) {
                    break;
                }
                WSASend(client.socket, (LPWSABUF) client.wsabuf.buf, client.wsabuf.len, &dw_size_sent, client.flag, &client.wol, NULL);
#ifdef DEBUG
                puts("sent back to client");
                assert(WSAGetLastError() != WSA_IO_PENDING);
#endif // DEBUG
            }
            spin_lock(p_keyholder_send);
            ResetEvent(send_evt);
            spin_unlock(p_keyholder_send);
        }
    }
    return;
}

#endif