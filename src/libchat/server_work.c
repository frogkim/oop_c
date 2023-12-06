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

    HANDLE evt = NULL;
    DWORD dw_event;
    node_t client;

    while (TRUE) {
        dw_event = WaitForSingleObject(p_param->evt, INFINITE);
#ifdef DEBUG
        printf("wake up work thread: %d\n", dw_event);
        printf("Last Error: %d\n", GetLastError());
#endif // DEBUG

        if (&p_param->terminate) {
            break;
        }
        while (TRUE) {
            if (p_work->size > 0) {
                p_work->get_front(p_work, &client);
#ifdef DEBUG
                puts("work thread retrieved data");
#endif // DEBUG
                // working



                // stack sending queue
                p_send->set_tail(p_send, &client);
                p_work_events->set_tail(p_work_events, &p_param->evt);
                p_send_events->get_front(p_send_events, &evt);
                SetEvent(evt);
#ifdef DEBUG
                puts("ready for replying to client");
#endif // DEBUG
                // end working
            } else {
                break;
            }
        }
    }
}

#endif