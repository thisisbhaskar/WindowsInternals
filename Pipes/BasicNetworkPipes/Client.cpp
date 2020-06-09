#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <sstream>

const int MESSAGE_SIZE = 4;

int main()
{
    auto l_continue = true;

    /* Connect to pipe */
    HANDLE l_pipe = INVALID_HANDLE_VALUE;
    if (l_continue)
    {
        LPCWSTR l_pipe_name = L"\\\\192.168.1.2\\pipe\\pipe111";
        std::wcout << "Connecting to pipe : " << l_pipe_name << std::endl;
        l_pipe = CreateFile(l_pipe_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == l_pipe)
        {
            std::wcout << "Failed to connect to pipe : " << l_pipe_name << " Error : " << GetLastError() << std::endl;
            l_continue = false;
        }
    }

    /* Read from pipe */
    while (l_continue)
    {
        wchar_t l_msg[MESSAGE_SIZE] = { 0 };
        DWORD l_bytes_read = 0;
        l_continue = ReadFile(l_pipe, &l_msg, MESSAGE_SIZE, &l_bytes_read, NULL);
        if (l_continue)
        {
            std::wcout << l_msg;
            std::cout << std::endl;
        }
        else if (ERROR_MORE_DATA == GetLastError())
        {
            std::wcout << l_msg;
            l_continue = true;
        }
        else
        {
            std::wcout << "Failed to read from pipe. Error : " << GetLastError();
        }
    }

    return 0;
}
