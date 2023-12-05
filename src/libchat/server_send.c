#include "libchat_server.h"

#ifdef LINUX
#else
void _func_send_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(pParam);
    p_send_parameter p_param = (p_send_parameter)pParam;
    p_queue_t p_send = p_param->q_send;
    p_queue_t p_send_events = p_param->q_send_events;
    HANDLE evt = p_param->evt;

    node_t          client;
    p_node_t        p_client = &client;

    DWORD dw_event;
    DWORD dw_size_sent;
    while (TRUE) {
        dw_event = WaitForSingleObject(evt, INFINITE);
        if (p_param->terminate) {
            break;
        }

        if (p_send->size > 0) {
            p_send->get_front(p_send, &p_client);
            if (p_client == NULL) {
                break;
            }
            WSASend(client.socket, (LPWSABUF)client.wsabuf.buf, client.wsabuf.len, &dw_size_sent, client.flag, client.p_wol, NULL);
            p_send_events->set_tail(p_send_events, &evt);
#ifdef DEBUG
            puts("sent back to client");
            assert(WSAGetLastError() != WSA_IO_PENDING);
#endif // DEBUG
        } 
#ifdef DEBUG
        puts("_func_send_server thread wake up unknown reason");
        assert(FALSE);
#endif // DEBUG
    }
    return;
}

#endif