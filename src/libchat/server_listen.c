#include "libchat_internal.h"

#ifdef LINUX
#else
DWORD CALLBACK _func_listen_server(LPVOID pParam)
{
    p_Server_original_t self = pParam;
    SOCKET          s_listen = self->_s_listen;
    HANDLE          h_iocp = self->_h_iocp;
    p_node_t        clients = self->_nodes_client;
    p_queue_t       p_seat_queue = self->_q_prior_seats_uint32;
    p_queue_t       p_recv_queue = self->_q_recv_client;
    PKEYHOLDER      p_keyholder_seat = &self->_keyholder_seat;
    PKEYHOLDER      p_keyholder_recv = &self->_keyholder_recv;

    SOCKET			h_client;
    SOCKADDR		ClientAddr;
    int				nAddrSize = sizeof(SOCKADDR);
    uint32_t        index = 0;
    BOOL            result = FALSE;
#ifdef DEBUG
    puts("func_thread_accept started.");
#endif
    // TODO: WSAaccept function conditionally accepts. Find it.
    while ((h_client = accept(s_listen, &ClientAddr, &nAddrSize)) != INVALID_SOCKET)
    {
        if (p_seat_queue->size > 0) {
            result = p_seat_queue->get_front(p_seat_queue, &index);
#ifdef DEBUG
            assert(result);
#endif // DEBUG
            spin_lock(p_keyholder_seat);
            index = p_seat_queue->size;
            p_seat_queue->size++;
            spin_unlock(p_keyholder_seat);
        }
        clients[index].socket = h_client;
        memset(clients[index].wsabuf.buf, 0, self->size_buffer);
        memset(&clients[index].wol, 0, sizeof(WSAOVERLAPPED));
        clients[index].n_recv = 0;
        clients[index].flag = 0;

        CreateIoCompletionPort((HANDLE)clients[index].socket, h_iocp, (ULONG_PTR)&clients[index], 0);
        WSARecv(clients[index].socket,
            &clients[index].wsabuf,
            1,                          // number of wsabuf
            &clients[index].n_recv,
            &clients[index].flag,       // flag for modifying the behavior of the WSARecv
            &clients[index].wol,
            NULL                        // callback
        );
#ifdef DEBUG
        puts("client connected");
        assert(WSAGetLastError() == WSA_IO_PENDING);
#endif // DEBUG
    }



    return 0;
}

#endif