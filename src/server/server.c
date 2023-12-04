#include <stdio.h>
#include "libchat.h"
#include <Windows.h>
#include <stdlib.h>
int main(int argv, char** argc)
{
    HMODULE hModule = LoadLibrary(TEXT("libchat.dll"));
    if (hModule == NULL) {
        puts("Failed to read dll.");
        return 0;
    }
    p_Server_t(__stdcall * CreateServer_or_null)(unsigned int, unsigned int)
    = (p_Server_t(__stdcall*)(unsigned int n_client, unsigned int buffer_size))
        GetProcAddress(hModule, "CreateServer_or_null");
    p_Server_t s = CreateServer_or_null(100, 8192);
    if (!s->init(s)) {
        printf("Failed to init");
    }

    puts("Server start.");

    char c;
    while (1) {
        c = getchar();
        if (c == 'q') {
            break;
        }
    }

    s->deinit(s);
}
