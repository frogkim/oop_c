#include <stdio.h>
#include "libchat.h"
#include <Windows.h>
#include <stdlib.h>
#define NODE_BUFFER_SIZE 8192
#define EMPTY_BUFFER(ch) while(ch != EOF && ch != '\n') { ch = getchar(); } 
#pragma warning(disable : 4996)

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

    char buf[NODE_BUFFER_SIZE];
    memset(buf, 0, NODE_BUFFER_SIZE);
    int continue_writing = 0;
    int index = 0;
    while (1) {
        printf("Enter command: ");
        char ch = 0;
        ch = getchar();
        switch (ch) {
        case 'q':
            puts("quit");
            goto END;

        case 'c':
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
        case 'i':
            printf("you enter: ");
            EMPTY_BUFFER(ch);
            ch = getchar();
            for (int index = 0; index < NODE_BUFFER_SIZE; index++) {
                buf[index] = ch;
                if (index == NODE_BUFFER_SIZE) {
                    break;
                }
                ch = getchar();
                if (ch == EOF || ch == '\n') {
                    buf[index + 1] = '\n';
                    break;
                }
            } 
            
            break;

        case 's':
            c->send_async(c, buf);
            break;

        case 'r':
            memset(buf, 0, 512);
            c->recv_async(c, buf);
            break;

        case 'p':
            printf("%s", buf);
            break;
        } // end switch
        EMPTY_BUFFER(ch);
    } // end while
END:
    c->deinit(c);
}
