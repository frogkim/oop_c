#include "libchat_server.h"


#ifdef LINUX
#else
void _func_iocp_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(work);
    p_Server_original_t self = pParam;

    HANDLE          h_iocp = self->_h_iocp;
    p_queue_t       p_seat = self->_q_prior_seats_uint32;
    p_queue_t       p_work = self->_q_work;
    p_queue_t       p_work_events = self->_q_work_events;

    HANDLE          evt = NULL;
    DWORD			size_transfer = 0;
    //node_t client;
    //p_node_t p_client = &client;

    WSAOVERLAPPED wol;
    memset(&wol, 0, sizeof(WSAOVERLAPPED));
    LPWSAOVERLAPPED p_wol = &wol;

    p_node_t p_client = NULL;

    BOOL			result;
    while (TRUE) {
        // TODO: investigate about result
        //result = GetQueuedCompletionStatus(h_iocp, &size_transfer, (PULONG_PTR)&p_client, &p_client->p_wol, INFINITE);
        result = GetQueuedCompletionStatus(h_iocp, &size_transfer, (PULONG_PTR)&p_client, &p_wol, INFINITE);
#ifdef DEBUG
        puts("Received from client");
#endif // DEBUG
        if (result == TRUE) {
            if (size_transfer > 0) {
                // normal. create iocp again
                // queue has its own critical section
                p_work->set_tail(p_work, p_client);
                while (!p_work_events->get_front(p_work_events, &evt)) {
                    // no waiting thread
                    Sleep(100);
                }
                SetEvent(evt);
                WSARecv(p_client->socket, &p_client->wsabuf, 1, &p_client->n_recv, &p_client->flag, p_client->p_wol, NULL);
#ifdef DEBUG
                int result = WSAGetLastError();
                assert(WSAGetLastError() == WSA_IO_PENDING);
#endif // DEBUG

            } else {
                // client try to close handle
#ifdef DEBUG
                printf("client: %d is disconnected correctly.\n", p_client->index);
#endif // DEBUG
                shutdown(p_client->socket, SD_BOTH);
                closesocket(p_client->socket);
                p_seat->set_tail(p_seat, &p_client->index);
            }
        } else {
            // disconnected
#ifdef DEBUG
            printf("client: %d is disconnected unexpectedly.\n", p_client->index);
#endif // DEBUG
            shutdown(p_client->socket, SD_BOTH);
            closesocket(p_client->socket);
            p_seat->set_tail(p_seat, &p_client->index);

            //if (p_wol == NULL) {
            //    // failed to get packet or IOCP handle closed
            //} else {
            //    // disconnected by server or others
            //}
        }
    }
#ifdef DEBUG
    puts("IOCP functions ends");
#endif // DEBUG
}

#endif