#include "libchat_client.h"

#ifdef LINUX
#else
DWORD CALLBACK _func_recv_client(LPVOID pParam)
{
    p_Client_original_t     self = pParam;
    p_queue_t               p_recv = self->_q_recv; 
    
    //DWORD dw_event;
    DWORD dw_size_recv = 0;
    DWORD flag = 0;

    CHAR buffer[NODE_BUFFER_SIZE];
    memset(buffer, 0, NODE_BUFFER_SIZE);

    while (TRUE) {
        if (self->_terminate || self->_isstop) {
            break;
        }
        while (TRUE) {
            // synchronized receive
            dw_size_recv = recv(self->_socket, buffer, NODE_BUFFER_SIZE, 0);
            if (dw_size_recv == 0) {
                break;
            }
#ifdef DEBUG
            puts("Received.");
            //int result = WSAGetLastError();
            //assert(result == WSA_IO_PENDING);
#endif // DEBUG
            p_recv->set_tail(p_recv, buffer);
            if (self->_terminate || self->_isstop) {
                goto FINISH_FUNC_RECV_CLIENT;
            }
        }
    }
FINISH_FUNC_RECV_CLIENT:
    safe_release(buffer);
    return 0;
}

DWORD CALLBACK _func_send_client(LPVOID pParam)
{
    p_Client_original_t self = pParam;
    p_queue_t       p_send = self->_q_send;
    HANDLE          send_evt = self->_evt_send;
    PKEYHOLDER      p_keyholder_send = &self->_send_keyholder;

    CHAR buffer[NODE_BUFFER_SIZE];
    memset(buffer, 0, NODE_BUFFER_SIZE);

    DWORD dw_event;
    int dw_size_sent = 0;
    int flag = 0;
    while (TRUE) {
        dw_event = WaitForSingleObject(send_evt, INFINITE);
        spin_lock(p_keyholder_send);
        ResetEvent(send_evt);
        spin_unlock(p_keyholder_send);

        if (self->_terminate || self->_isstop) {
            goto FINISH_FUNC_SEND_CLIENT;
        }
        if (p_send->size > 0) {
            if (p_send->get_front(p_send, buffer)) {
                //dw_size_sent = send(self->_socket, buffer, NODE_BUFFER_SIZE, flag);
                flag = 0;
                dw_size_sent = send(self->_socket, buffer, NODE_BUFFER_SIZE, flag);
#ifdef DEBUG
                if (dw_size_sent < 0) {
                    int result = GetLastError();
                    printf("sent error: %d\n", result);
                } else {
                    printf("data are sent: %d bytes\n", dw_size_sent);
                }
#endif // DEBUG
            }
        } else {
#ifdef DEBUG
            puts("No data in send queue");
#endif // DEBUG
        }
    }
FINISH_FUNC_SEND_CLIENT:
    safe_release(buffer);
    return 0;
}


#endif

