#include "libchat_client.h"

#ifdef LINUX
#else
DWORD CALLBACK _func_recv_client(LPVOID pParam)
{
    p_Client_original_t self = pParam;
    p_queue_t       p_recv = self->_q_recv; 
    HANDLE          recv_evt = self->_evt_recv;
    PKEYHOLDER      p_keyholder_recv = &self->_keyholder_recv;
    
    //DWORD dw_event;
    DWORD dw_size_recv = 0;
    DWORD flag = 0;

    WSABUF wsabuf;
    memset(&wsabuf, 0, sizeof(WSABUF));
    wsabuf.buf = malloc(self->size_buffer);
    memset(wsabuf.buf, 0, self->size_buffer);
    wsabuf.len = self->size_buffer;

    //WSAOVERLAPPED wol;
    //memset(&wol, 0, sizeof(WSAOVERLAPPED));
    //LPWSAOVERLAPPED p_wol = &wol;

    while (TRUE) {
        //dw_event = WaitForSingleObject(recv_evt, INFINITE);
        if (self->_terminate || self->_isstop) {
            break;
        }
        while (TRUE) {
            // synchronized receive
            dw_size_recv = recv(self->_socket, wsabuf.buf, self->size_buffer, 0);
            //WSARecv(self->_socket, &wsabuf, 1, &dw_size_recv, &flag,  &wol, NULL);
            if (dw_size_recv == 0) {
                break;
            }
#ifdef DEBUG
            puts("Received.");
            //int result = WSAGetLastError();
            //assert(result == WSA_IO_PENDING);
#endif // DEBUG
            p_recv->set_tail(p_recv, self->_buffer);
            if (self->_terminate || self->_isstop) {
                goto FINISH_FUNC_RECV_CLIENT;
            }
        }
    }
FINISH_FUNC_RECV_CLIENT:
    safe_release(wsabuf.buf);
    return 0;
}

DWORD CALLBACK _func_send_client(LPVOID pParam)
{
    p_Client_original_t self = pParam;
    p_queue_t       p_send = self->_q_send;
    HANDLE          send_evt = self->_evt_send;
    PKEYHOLDER      p_keyholder_send = &self->_keyholder_send;

    WSABUF wsabuf;
    wsabuf.buf = malloc(self->size_buffer);
    wsabuf.len = self->size_buffer;

    WSAOVERLAPPED wol;
    memset(&wol, 0, sizeof(WSAOVERLAPPED));
    LPWSAOVERLAPPED p_wol = &wol;
    
    DWORD dw_event;
    DWORD dw_size_sent;
    DWORD flag = 0;
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
                if (p_send->get_front(p_send, wsabuf.buf)) {
                    WSASend(self->_socket, &wsabuf, 1, &dw_size_sent, flag, p_wol, NULL);
#ifdef DEBUG
                    int result = WSAGetLastError();
                    puts("WSASend tried.");
                    //assert(WSAGetLastError() == WSA_IO_PENDING);
#endif // DEBUG
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
    safe_release(wsabuf.buf);
    return 0;
}


#endif

