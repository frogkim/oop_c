#include "libchat_client.h"
#ifdef LINUX
#else
static BOOL set_ip_port(PSOCKADDR_IN p_sockaddr, PCHAR ip_or_null, USHORT port);


BOOL _setup_client(p_Client_t _self, PCHAR ip, USHORT port)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    return set_ip_port(&self->_addrsvr_in, ip, port);
}

BOOL _connect_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    WSADATA wsa = { 0 };
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        puts("Failed to initialize winsock.");
        return 0;
    }

    self->_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (self->_socket == INVALID_SOCKET) {
        return FALSE;
    }

    if (connect(self->_socket, (SOCKADDR*)&self->_addrsvr_in, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
        return FALSE;
    }
    self->resume((p_Client_t)self);
    self->_isconnect = TRUE;

    CreateThread(NULL, 0, _func_recv_client, self, 0, NULL);
    CreateThread(NULL, 0, _func_send_client, self, 0, NULL);
    return TRUE;
}

BOOL _disconnect_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    if (!self->_isstop) {
        self->stop((p_Client_t)self);
    }
    CloseHandle(self->_h_recv_thread);
    CloseHandle(self->_h_send_thread);
    shutdown(self->_socket, SD_BOTH);
    closesocket(self->_socket);
    self->_isconnect = FALSE;
    WSACleanup();
    return TRUE;
}

VOID _resume_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    self->_isstop = FALSE;
}

VOID _stop_client(p_Client_t _self)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    PKEYHOLDER p_key = &self->_send_keyholder;

    self->_isstop = TRUE;
}

VOID _recv_async_client(p_Client_t _self, PVOID data_out)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    if (self->_q_recv->size == 0) {
        puts("No data to read.");
    } else {
        self->_q_recv->get_front(self->_q_recv, data_out);
        puts("read data");
    }
}

VOID _send_async_client(p_Client_t _self, PVOID data_in)
{
    p_Client_original_t self = (p_Client_original_t)_self;
    PKEYHOLDER p_key = &self->_send_keyholder;

    self->_q_send->set_tail(self->_q_send, data_in);
    spin_lock(p_key);
    SetEvent(self->_evt_send);
    spin_unlock(p_key);
    puts("sent data.");
}

BOOL set_ip_port(PSOCKADDR_IN p_sockaddr, PCHAR ip_or_null, USHORT port)
{
    memset(p_sockaddr, 0, sizeof(SOCKADDR_IN));
    p_sockaddr->sin_family = AF_INET;
    if (!inet_pton(AF_INET, ip_or_null, &p_sockaddr->sin_addr)) {
        return FALSE;
    }
    p_sockaddr->sin_port = htons(port);
    return TRUE;
}
#endif
