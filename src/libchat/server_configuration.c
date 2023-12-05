#include "libchat_server.h"
#ifdef LINUX
#else

VOID _stop_server(p_Server_t _self)
{
    p_Server_original_t self = (p_Server_original_t)_self;
    self->_stop = TRUE;
}


VOID _resume_server(p_Server_t _self)
{
    p_Server_original_t self = (p_Server_original_t)_self;
    self->_stop = FALSE;
}
#endif
