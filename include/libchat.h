#ifndef LIBCHAT_H_
#define LIBCHAT_H_


#ifdef LINUX
// --- Linux ---
#define ALIGN(x) __attribute__ ((aligned(x)))

#ifdef EXPORT
#define LIBUTILS __attribute__((visibility("default")))
#else
#define LIBUTILS
// --- Linux ---
#endif

#else
// --- Windows ---

#ifdef EXPORT
#define LIBCHAT __declspec(dllexport)
#else
#define LIBCHAT __declspec(dllimport)
#endif
// --- Windows ---
#endif
typedef struct _server {
    // public
    int (*init)(struct _server*);
    void (*stop)(struct _server*);
    void (*resume)(struct _server*);
    int (*deinit)(struct _server*);
} Server_t, *p_Server_t;

typedef struct _client {
    // public
    int (*setup)(struct _client*, char* ip, unsigned short port);
    int (*init)(struct _client*);
    int (*deinit)(struct _client*);
    int (*connect)(struct _client*);
    int (*disconnect)(struct _client*);
    void (*resume)(struct _client*);
    void (*stop)(struct _client*);
    void (*recv_async)(struct _client*, void* data_out);
    void (*send_async)(struct _client*, void* data_in);

} Client_t, * p_Client_t;

extern LIBCHAT p_Server_t __stdcall
CreateServer_or_null(unsigned int n_clients, unsigned int buffer_size);

// Create -> Optional: Setup -> Init -> Connect
extern LIBCHAT p_Client_t __stdcall
CreateClient_or_null(unsigned int buffer_size); 


#endif
