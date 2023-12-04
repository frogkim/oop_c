#include "libchat_internal.h"


#ifdef LINUX
#else
void _func_iocp_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(work);
    p_Server_original_t self = pParam;

    HANDLE          h_iocp = self->_h_iocp;
    p_queue_t       p_seat_queue = self->_q_prior_seats_uint32;
    p_queue_t       p_recv_queue = self->_q_recv_client;
    p_queue_t       p_send_queue = self->_q_send_client;
    HANDLE          recv_evt = self->_evt_recv;
    HANDLE          send_evt = self->_evt_send;
    PKEYHOLDER      p_keyholder_recv = &self->_keyholder_recv;

    DWORD			size_transfer = 0;
    node_t client;
    p_node_t p_client = &client;
    
    BOOL			result;
    while (TRUE) {
        // TODO: investigate about result
        result = GetQueuedCompletionStatus(h_iocp, &size_transfer, (PULONG_PTR)p_client, (LPOVERLAPPED*)&p_client->wol, INFINITE);

        if (result == TRUE) {
            if (size_transfer > 0) {
                // normal. create iocp again
                // queue has its own critical section
                p_recv_queue->set_tail(p_send_queue, &client);
                spin_lock(p_keyholder_recv);
                SetEvent(recv_evt);
                spin_unlock(p_keyholder_recv);
                WSARecv(client.socket, &client.wsabuf, 1, &client.n_recv, &client.flag, &client.wol, NULL);
#ifdef DEBUG
                assert(WSAGetLastError() != WSA_IO_PENDING);
#endif // DEBUG
            } else {
                // client try to close handle
#ifdef DEBUG
                printf("client: %d is disconnected correctly.\n", client.index);
#endif // DEBUG
                shutdown(client.socket, SD_BOTH);
                closesocket(client.socket);
                p_seat_queue->set_tail(p_seat_queue, &client.index);
            }
        } else {
            // disconnected
#ifdef DEBUG
            printf("client: %d is disconnected unexpectedly.\n", client.index);
#endif // DEBUG
            p_seat_queue->set_tail(p_seat_queue, &client.index);

            //if (p_wol == NULL) {
            //    // failed to get packet or IOCP handle closed
            //} else {
            //    // disconnected by server or others
            //}
        }
    }
}

#endif