#include "stdafx.h"
#include <afxwin.h>
#include <Windows.h>
#include <string>
#include <memory>
#include <iostream>
#include <ntstatus.h>
#include <synchapi.h>
#include <strsafe.h>

using namespace std;

void IOCompletionRoutine(DWORD p_error,
                         DWORD p_bytes_transferred,
                         OVERLAPPED* p_overlapped)
{
    int a  = 0;
    cout << a;
}

void main()
{
    bool l_continue = true;
    HANDLE l_file_handle = CreateFile(
        L"C:\\temp.txt",
        FILE_ALL_ACCESS,
        0,
        NULL, 
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (INVALID_HANDLE_VALUE == l_file_handle)
    {
        l_continue = false;
    }

    if (l_continue)
    {
        PVOID l_buffer = malloc(100);
        OVERLAPPED l_overlapped;
        RtlZeroMemory(&l_overlapped, sizeof(l_overlapped));
        bool l_status = ReadFileEx(l_file_handle,
                                   l_buffer,
                                   100,
                                   &l_overlapped,
                                   (LPOVERLAPPED_COMPLETION_ROUTINE)IOCompletionRoutine);
        if (l_status) {
            DWORD l_wait_status = WaitForSingleObjectEx(l_file_handle, INFINITE, true);
            if (0x000000C0L == l_wait_status)
            {
                cout << "Got the data";
            } 
            else
            {
                l_continue = false;
            }
        }
           
        if (INVALID_HANDLE_VALUE != l_file_handle)
        {
            CloseHandle(l_file_handle);
        }

        ::free(l_buffer);
    }
}
