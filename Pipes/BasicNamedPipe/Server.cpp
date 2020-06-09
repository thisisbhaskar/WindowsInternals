#include "stdafx.h"
#include <Windows.h>
#include <iostream>

int main()
{
    auto l_continue = true;
    LPCWSTR l_pipe_name = L"\\\\.\\pipe\\pipe111";
  
    /* Create pipe */
    HANDLE l_pipe = INVALID_HANDLE_VALUE;
    if (l_continue)
    {
        std::wcout << "Creating pipe : " << l_pipe_name << std::endl;
        l_pipe = CreateNamedPipe(l_pipe_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE, 1, 2048, 2048, 0, nullptr);
        if (INVALID_HANDLE_VALUE == l_pipe)
        {
            std::wcout << "Failed to create pipe : " << l_pipe_name << std::endl;
        }
    }

    /* Wait for client to connect */
    if (l_continue)
    {
        std::wcout << "Connecting to pipe : " << l_pipe_name << std::endl;
        l_continue = ConnectNamedPipe(l_pipe, nullptr);
        if (!l_continue)
        {
            std::wcout << "Failed to connect to pipe :  " << l_pipe << std::endl;
        }
    }

    /* Write to pipe */
    if (l_continue)
    {
        wchar_t l_msg[] = L"Hello..";
        DWORD l_bytes_length = lstrlen(l_msg) * 2;
        while (l_continue)
        {
            DWORD l_byes_written = 0;
            std::wcout << "Sending: " << l_msg << std::endl;
            l_continue = WriteFile(l_pipe, l_msg, l_bytes_length, &l_byes_written, nullptr);

            Sleep( 2 * 1000);
        }

        /* Disconnect pipe */
        DisconnectNamedPipe(l_pipe);
    }

    /* Close pipe */
    if (INVALID_HANDLE_VALUE != l_pipe)
    {
        CloseHandle(l_pipe);
    }

    return 0;
}
