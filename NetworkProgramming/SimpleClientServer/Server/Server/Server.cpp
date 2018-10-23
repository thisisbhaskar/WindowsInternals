// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdlib.h>
#include <stdio.h>

using namespace std;

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8081"

/**
* Thread to handle new client connections
*/
DWORD WINAPI ConnectionHandlerThread(LPVOID p_param)
{
    SOCKET ClientSocket = (SOCKET)p_param;

    /* Receive until the peer shuts down the connection */
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int l_result = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (l_result > 0)
    {
        printf("Bytes received: %d\n", l_result);

        /* Echo the buffer back to the sender */
        char *sendbuf1 =
"HTTP/1.1 200 OK\r\n\
Connection : keep-alive\r\n\
Content-Type : text/html; charset=utf-8\r\n";

        int iSendResult = send(ClientSocket, sendbuf1, (int)strlen(sendbuf1), 0);
        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
        }
        printf("Bytes sent: %d\n", iSendResult);

        char *sendbuf2 =
"Content-Length : 51\r\n\
Transfer-Encoding: identity\r\n\
\r\n\
<html><body><h1>My First Heading</h1></body></html>";

        iSendResult = send(ClientSocket, sendbuf2, (int)strlen(sendbuf2), 0);
        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
        }
        printf("Bytes sent: %d\n", iSendResult);
    }
    else if (l_result == 0)
    {
        printf("Connection closing...\n");
    }
    else
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    /* Shutdown the connection since we're done */
    l_result = shutdown(ClientSocket, SD_SEND);
    if (l_result == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
    }

    /* Socket cleanup */
    closesocket(ClientSocket);

    return 1;
}

int main()
{
    /* Initialize Winsock */
    WSADATA wsaData;
    int l_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (l_result != 0)
    {
        printf("WSAStartup failed with error: %d\n", l_result);
        return 1;
    }

    /* Resolve the server address and port */
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *result = NULL;
    l_result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (l_result != 0)
    {
        printf("getaddrinfo failed with error: %d\n", l_result);
        WSACleanup();
        return 1;
    }

    /* Create a SOCKET for connecting to server */
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    /* Setup the TCP listening socket */
    l_result = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (l_result == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    /* Free address result */
    freeaddrinfo(result);

    /* Listen */
    l_result = listen(ListenSocket, SOMAXCONN);
    if (l_result == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    while (true)
    {
        /* Accept a client socket */
        SOCKET ClientSocket = INVALID_SOCKET;
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        else
        {
            DWORD l_thread_id;
            HANDLE  l_handle = CreateThread(NULL, 0, ConnectionHandlerThread, (LPVOID)ClientSocket, 0, &l_thread_id);
            if (l_handle == NULL)
            {
                /* Close client socket */
                closesocket(ClientSocket);
            }
            else
            {
                /* Close handle */
                CloseHandle(l_handle);
            }
        }
    }

    /* WSA Cleanup */
    WSACleanup();

    return 0;
}

