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
    p_Client_t(__stdcall * CreateClient_or_null)(unsigned int buffer_size)
        = (p_Client_t(__stdcall*)(unsigned int buffer_size))
        GetProcAddress(hModule, "CreateClient_or_null");
    p_Client_t c = CreateClient_or_null(8192);
    if (c == NULL) {
        puts("Failed to create client");
        return 1;
    }

    if (!c->init(c)) {
        printf("Failed to init");
    }
    printf("This is client\n");
    if (!c->setup(c, "127.0.0.1", 25000)) {
        puts("Failed to setup");
        return 1;
    }


    char buf[8192];
    memset(buf, 0, 8192);
    
    while (1) {
        char ch = getchar();
        switch (ch) {
        case 'q': 
            puts("quit");
            goto END;

        case 'c':
            puts("tried connect");
            if (!c->connect(c)) {
                puts("Failed to connect");
                return 1;
            }
            puts("Success to connect");
            break;
        case 'd':
            puts("tried disconnect");
            if (!c->disconnect(c)) {
                puts("Failed to disconnect");
                return 1;
            }
            puts("Success to disconnect");
            break;


        case 's':
            printf("you enter: ");
            int n = scanf("%s", buf);
            c->send_async(c, buf);
            puts("Sent asynchronously");
            break;

        case 'r':
            memset(buf, 0, 8192);
            c->recv_async(c, buf);
            if (buf[0] == 0) {
                puts("failed to get received data");
            } else {
                printf("you receive: %s\n", buf);
            }
            break;
        }

    }
END:






    c->deinit(c);
}
