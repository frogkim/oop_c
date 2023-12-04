#include "libchat_internal.h"



#ifdef LINUX
#else
DWORD CALLBACK _func_recv_client(LPVOID pParam)
{
    p_Client_original_t self = pParam;
    p_queue_t       p_recv = self->_q_recv; 
    //HANDLE          recv_evt = self->_evt_recv;
    PKEYHOLDER      p_keyholder_recv = &self->_keyholder_recv;
    
    //DWORD dw_event;
    DWORD dw_size_recv = 0;
    DWORD flag = 0;

    WSABUF wsabuf;
    CHAR* memblock = (CHAR*)malloc(self->size_buffer);
    wsabuf.buf = memblock;
    wsabuf.len = self->size_buffer;

    WSAOVERLAPPED wol;
    memset(&wol, 0, sizeof(WSAOVERLAPPED));

    while (TRUE) {
        while (TRUE) {
            //dw_size_recv = recv(self->_socket, self->_buffer, self->size_buffer, 0);
            WSARecv(self->_socket, &wsabuf, 1, &dw_size_recv, &flag,  &wol, NULL);
            if (dw_size_recv == 0) {
                break;
            }
#ifdef DEBUG
            puts("Recv tried.");
            int result = WSAGetLastError();
            assert(result == WSA_IO_PENDING);
#endif // DEBUG
            p_recv->set_tail(p_recv, self->_buffer);
            if (self->_terminate || self->_isstop) {
                goto FINISH_FUNC_RECV_CLIENT;
            }
            //spin_lock(p_keyholder_recv);
            //SetEvent(recv_evt);
            //spin_unlock(p_keyholder_recv);
        }
        //dw_event = WaitForSingleObject(recv_evt, INFINITE);
        //if (self->_terminate || self->_isstop) {
        //    break;
        //}
    }
FINISH_FUNC_RECV_CLIENT:
    safe_release(memblock);
    return 0;
}

DWORD CALLBACK _func_send_client(LPVOID pParam)
{
    p_Client_original_t self = pParam;
    node_t          server;
    p_node_t        p_server = &server;
    memset(p_server, 0, sizeof(node_t));
    p_queue_t       p_send = self->_q_send;
    HANDLE          send_evt = self->_evt_send;
    PKEYHOLDER      p_keyholder_send = &self->_keyholder_send;

    DWORD dw_event;
    DWORD dw_size_sent;
    while (TRUE) {
        dw_event = WaitForSingleObject(send_evt, INFINITE);
        if (self->_terminate || self->_isstop) {
            break;
        }
        while (TRUE) {
            if (self->_terminate || self->_isstop) {
                goto FINISH_FUNC_SEND_CLIENT;
            }
            if (p_send->size > 0) {
                if (p_send->get_front(p_send, p_server)) {
                    WSASend(server.socket, (LPWSABUF)server.wsabuf.buf, server.wsabuf.len, &dw_size_sent, server.flag, &server.wol, NULL);
#ifdef DEBUG
                    puts("WSASend tried.");
                    assert(WSAGetLastError() == WSA_IO_PENDING);
#endif // DEBUG
                } else {
                    
                }
            } else {
                break;
            }
        }
        spin_lock(p_keyholder_send);
        ResetEvent(send_evt);
        spin_unlock(p_keyholder_send);
    }
FINISH_FUNC_SEND_CLIENT:
    return 0;
}


#endif

