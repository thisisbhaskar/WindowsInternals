#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <tuple>
using namespace std;

typedef HMODULE (*LoadLibraryLoc)(LPCSTR lpLibFileName);

void _tmain(int argc, TCHAR *argv[])
{
    bool l_continue = true;

    /* Process Handle */
    HANDLE l_proc_handle = NULL;
    if (l_continue)
    {
        l_proc_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, 2528);
        if (NULL == l_proc_handle)
        {
            cout << GetLastError();
            l_continue = false;
        }
    }

    /* Allocate Memory */
    LPVOID l_alloc_ptr = NULL;
    if (l_continue)
    {
        l_alloc_ptr = VirtualAllocEx(l_proc_handle, NULL, 500, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (NULL == l_alloc_ptr)
        {
            cout << GetLastError();
            l_continue = false;
        }
    }

    /* Copy DLL Name */
    if (l_continue)
    {
        wchar_t* l_dll_path = L"C:\\Users\\bhasker\\Desktop\\Projects\\WindowsInternals\\DLLInjection\\TargetDll\\x64\\Release\\TargetDll.dll";
        size_t l_bytes_total = (wcslen(l_dll_path) * sizeof(wchar_t)) + sizeof(wchar_t);
        size_t l_bytes_written = 0;
        WriteProcessMemory(l_proc_handle, l_alloc_ptr, l_dll_path, l_bytes_total,  &l_bytes_written);
        if (l_bytes_total != l_bytes_written)
        {
            std::cout << GetLastError();
            l_continue = false;
        }
    }

    /* Get Load library address */
    LoadLibraryLoc l_func_address = NULL;
    if (l_continue)
    {
        HMODULE l_handle = ::LoadLibrary(L"Kernel32.dll");
        if (l_handle)
        {
            l_func_address = (LoadLibraryLoc)GetProcAddress(l_handle, "LoadLibraryW");
            if (NULL == l_func_address)
            {
                cout << GetLastError();
                l_continue = false;
            }
        }
        else
        {
            cout << GetLastError();
            l_continue = false;
        }
    }

    /* Create remote thread */
    HANDLE l_remote_thread = NULL;
    if (l_continue)
    {
        DWORD l_thread_id;
        l_remote_thread = CreateRemoteThread(l_proc_handle, NULL, 0, (LPTHREAD_START_ROUTINE )l_func_address, l_alloc_ptr, 0, &l_thread_id);
        if (NULL == l_remote_thread)
        {
            cout << GetLastError();
        }
    }

    if (l_proc_handle)
    {
        CloseHandle(l_proc_handle);
    }

    if (l_alloc_ptr)
    {
        VirtualFree(l_alloc_ptr, 0, MEM_RELEASE);
    }

    if (l_remote_thread)
    {
        CloseHandle(l_remote_thread);
    }

    return;
}