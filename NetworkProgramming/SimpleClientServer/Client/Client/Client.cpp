#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

/* Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib */
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 8081

int __cdecl main(int argc, char **argv)
{
    /* Initialize Winsock */
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }


    /* Attempt to connect to an address until one succeeds */
    SOCKET ConnectSocket = INVALID_SOCKET;

    /* Create a SOCKET for connecting to server */
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    /* Connect to server */
    struct sockaddr_in saServer;
    saServer.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(saServer.sin_addr));
    saServer.sin_port = htons(DEFAULT_PORT);

    iResult = connect(ConnectSocket, (sockaddr *) &saServer, sizeof(saServer));
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    /* Send an initial buffer */
    char *sendbuf = 
"GET / HTTP/1.1\r\n\
Host: localhost\r\n\
Connection : keep - alive\r\n\
Cache - Control : no - cache\r\n\
User - Agent : Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 70.0.3538.67 Safari / 537.36\r\n\
Postman - Token : 026fb2fd - f52d - 748e - d17a - 5a481b915487\r\n\
Accept : */*\r\n\
Accept-Encoding: gzip, deflate, br\r\n\
Accept-Language: en-US,en;q=0.9,te;q=0.8,hi;q=0.7\r\n";

    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    /* Receive until the peer closes the connection */
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    do
    {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());
    }
    while (iResult > 0);


    /* shutdown the connection since no more data will be sent */
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    /* cleanup */
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}