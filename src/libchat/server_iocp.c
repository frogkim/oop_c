#include "libchat_server.h"


#ifdef LINUX
#else
void _func_iocp_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(work);
    p_Server_original_t self = pParam;

    HANDLE          h_iocp = self->_h_iocp;
    HANDLE          evt = NULL;
    PHANDLE         p_evt = &evt;
    p_queue_t       p_seat = self->_q_prior_seats_uint32;
    p_queue_t       p_work = self->_q_work;
    p_queue_t       p_work_events = self->_q_work_events;
    DWORD			size_transfer = 0;
    node_t client;
    p_node_t p_client = &client;
    
    BOOL			result;
    while (TRUE) {
        // TODO: investigate about result
        result = GetQueuedCompletionStatus(h_iocp, &size_transfer, (PULONG_PTR)&p_client, &p_client->p_wol, INFINITE);

        if (result == TRUE) {
            if (size_transfer > 0) {
                // normal. create iocp again
                // queue has its own critical section
                p_work->set_tail(p_work, &client);
                p_work_events->get_front(p_work_events, p_evt);
                SetEvent(*p_evt);
                WSARecv(client.socket, &client.wsabuf, 1, &client.n_recv, &client.flag, client.p_wol, NULL);
#ifdef DEBUG
                puts("Received from client");
                assert(WSAGetLastError() != WSA_IO_PENDING);
#endif // DEBUG
            } else {
                // client try to close handle
#ifdef DEBUG
                printf("client: %d is disconnected correctly.\n", client.index);
#endif // DEBUG
                shutdown(client.socket, SD_BOTH);
                closesocket(client.socket);
                p_seat->set_tail(p_seat, &client.index);
            }
        } else {
            // disconnected
#ifdef DEBUG
            printf("client: %d is disconnected unexpectedly.\n", client.index);
#endif // DEBUG
            shutdown(client.socket, SD_BOTH);
            closesocket(client.socket);
            p_seat->set_tail(p_seat, &client.index);

            //if (p_wol == NULL) {
            //    // failed to get packet or IOCP handle closed
            //} else {
            //    // disconnected by server or others
            //}
        }
    }
}

#endif