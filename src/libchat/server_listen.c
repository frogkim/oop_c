#include "libchat_server.h"

#ifdef LINUX
#else
DWORD CALLBACK _func_listen_server(LPVOID pParam)
{
    p_Server_original_t self = pParam;
    SOCKET          s_listen = self->_s_listen;
    HANDLE          h_iocp = self->_h_iocp;
    p_node_t        clients = self->_nodes_client;
    p_queue_t       p_seat_queue = self->_q_prior_seats_uint32;

    SOCKET			h_client;
    SOCKADDR		ClientAddr;
    int				nAddrSize = sizeof(SOCKADDR);
    uint32_t        index = 0;
    uint32_t        max_clients = 0;
    BOOL            result = FALSE;
#ifdef DEBUG
    puts("func_thread_accept started.");
#endif
    // TODO: WSAaccept function conditionally accepts. Find it.
    while ((h_client = accept(s_listen, &ClientAddr, &nAddrSize)) != INVALID_SOCKET)
    {
#ifdef DEBUG
        puts("client connected");
#endif // DEBUG


        if (p_seat_queue->size > 0) {
            p_seat_queue->get_front(p_seat_queue, &index);
        } else {
            index = max_clients;
            max_clients++;
        }

        clients[index].socket = h_client;
        memset(clients[index].wsabuf.buf, 0, self->size_buffer);
        memset(&clients[index].wol, 0, sizeof(WSAOVERLAPPED));
        clients[index].p_wol = &clients[index].wol;
        clients[index].n_recv = 0;
        clients[index].flag = 0;

        CreateIoCompletionPort((HANDLE)clients[index].socket, h_iocp, (ULONG_PTR)&clients[index], 0);
        WSARecv(clients[index].socket,
            &clients[index].wsabuf,
            1,                          // number of wsabuf
            &clients[index].n_recv,
            &clients[index].flag,       // flag for modifying the behavior of the WSARecv
            clients[index].p_wol,
            NULL                        // callback
        );
#ifdef DEBUG
        int result = WSAGetLastError();
        assert(WSAGetLastError() == WSA_IO_PENDING);
#endif // DEBUG
    }



    return 0;
}

#endif