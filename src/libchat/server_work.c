#include "libchat_server.h"

#ifdef LINUX
#else
void _func_work_server(PTP_CALLBACK_INSTANCE instance, PVOID pParam, PTP_WORK work)
{
    UNREFERENCED_PARAMETER(instance);
    UNREFERENCED_PARAMETER(work);
    p_work_parameter_t p_param = (p_work_parameter_t) pParam;

    p_queue_t p_work = p_param->q_work;
    p_queue_t p_send = p_param->q_send;
    p_queue_t p_work_events = p_param->q_work_events;
    p_queue_t p_send_events = p_param->q_send_events;

    HANDLE work_evt = NULL;
    //PHANDLE p_work_evt = &work_evt;

    HANDLE send_evt = NULL;
    //PHANDLE p_send_evt = &send_evt;

    p_work_events->get_front(p_work_events, &work_evt);
    p_work_events->set_tail(p_work_events, &work_evt);
    DWORD dw_event;
    node_t client;

    CHAR buffer[NODE_BUFFER_SIZE];

    while (TRUE) {
        dw_event = WaitForSingleObject(work_evt, INFINITE);
#ifdef DEBUG
        printf("[WORK] evt %p wake up\n", work_evt);
        if (GetLastError()) {
            printf("[WORK] evt %p, Last Error: %d\n", work_evt, GetLastError());
        }
#endif // DEBUG

        if (*p_param->terminate) {
            break;
        }

        if (p_work->size > 0) {
            p_work->get_front(p_work, &client);
#ifdef DEBUG
            printf("[WORK] %p retrieve index %d\n", work_evt, client.index);
#endif // DEBUG
        }
        // working

        memset(buffer, 0, NODE_BUFFER_SIZE);

        size_t len = strlen("Server received message - ");
        memcpy(buffer, "Server received message - ", len);
        size_t n_str = strlen(client.wsabuf.buf);
        strcpy_s(&buffer[len + 1], n_str, client.wsabuf.buf);
        client.wsabuf.len = len + n_str;
        memcpy(&client.wsabuf.buf[0], &buffer[0], client.wsabuf.len);

        // stack sending queue
        //p_send->set_tail(p_send, &client);
        p_send->set_tail(p_send, &client);

        p_work_events->set_tail(p_work_events, &work_evt);
        p_send_events->get_front(p_send_events, &send_evt);
        SetEvent(send_evt);
#ifdef DEBUG
        printf("[WORK] %p  - setup evt up %p\n", work_evt, send_evt);
#endif // DEBUG
        // end working

    }
}

#endif