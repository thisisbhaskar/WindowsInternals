// ParentProc.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <Windows.h>
#include <ntstatus.h>
#include <iostream>
#include <string>
#include <processthreadsapi.h>
#include <handleapi.h>
#include <accctrl.h>
#include <aclapi.h>
#include <ntsecapi.h>
#include <memoryapi.h>
#include <iomanip>
#include <Psapi.h>
#include <heapapi.h>


using namespace std;

string GetState(DWORD p_state)
{
    if (p_state == MEM_COMMIT)
    {
        return "MEM_COMMIT";
    }
    else if (p_state == MEM_FREE)
    {
        return "MEM_FREE";
    }
    else if (p_state == MEM_RESERVE)
    {
        return "MEM_RESERVE";
    }
    return "MEM_STATE_UNKNOWN";
}

string GetType(DWORD p_type)
{
    if (p_type == MEM_IMAGE)
    {
        return "MEM_IMAGE";
    }
    else if (p_type == MEM_MAPPED)
    {
        return "MEM_MAPPED";
    }
    else if (p_type == MEM_PRIVATE)
    {
        return "MEM_PRIVATE";
    }
    return "MEM_TYPE_UNKNOWN";
}

string GetProtection(DWORD p_protect)
{
    string l_protection;
    if (p_protect & PAGE_EXECUTE)
    {
        l_protection.append("PAGE_EXECUTE;");
    }
    else if (p_protect & PAGE_EXECUTE_READ)
    {
        l_protection.append("PAGE_EXECUTE_READ;");
    }
    else if (p_protect & PAGE_EXECUTE_READWRITE)
    {
        l_protection.append("PAGE_EXECUTE_READWRITE;");
    }
    else if (p_protect & PAGE_EXECUTE_WRITECOPY)
    {
        l_protection.append("PAGE_EXECUTE_WRITECOPY;");
    }
    else if (p_protect & PAGE_NOACCESS)
    {
        l_protection.append("PAGE_NOACCESS;");
    }
    else if (p_protect & PAGE_READONLY)
    {
        l_protection.append("PAGE_READONLY;");
    }
    else if (p_protect & PAGE_READWRITE)
    {
        l_protection.append("PAGE_READWRITE;");
    }
    else if (p_protect & PAGE_WRITECOPY)
    {
        l_protection.append("PAGE_WRITECOPY;");
    }
    return l_protection;
}

int main()
{

    char * l_ptr = 0x0;
    MEMORY_BASIC_INFORMATION l_mem_info;
    while (true)
    {
        SIZE_T l_size = VirtualQuery(l_ptr, &l_mem_info, sizeof(l_mem_info));
        if (l_size == 0)
        {
            break;
        }

        std::cout 
            << "Address : " << std::left << std::setw(20) << std::hex << (uint64_t)l_ptr
            << "  AlocBase: " << std::left << std::setw(20) << std::hex << (uint64_t)l_mem_info.AllocationBase
            << "  Size (KB): " << std::left << std::setw(20) << std::dec << (l_mem_info.RegionSize/1024)
            << "  State: " << std::left << std::setw(20) << GetState(l_mem_info.State)
            << "  Type: " << std::left << std::setw(20) << GetType(l_mem_info.Type)
            << "  Protect: " << std::left << std::setw(20) << GetProtection(l_mem_info.Protect)
            << std::endl;
        l_ptr = l_ptr + l_mem_info.RegionSize;
    }

    HMODULE hMods[1024];
    DWORD cbNeeded;
    unsigned int i;

    /* Get a list of all the modules in this process */
    if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];

            if (GetModuleFileNameEx(GetCurrentProcess(),
                                    hMods[i],
                                    szModName,
                                    sizeof(szModName) / sizeof(TCHAR)))
            {
                wcout << szModName << " Address: " << hMods[i] << std::endl;
            }
        }
    }

    /* Process Heaps */
    DWORD NumberOfHeaps;
    DWORD HeapsIndex;
    DWORD HeapsLength;
    HRESULT Result;
    PHANDLE aHeaps;
    SIZE_T BytesToAllocate;

    NumberOfHeaps = GetProcessHeaps(0, NULL);
    if (NumberOfHeaps == 0)
    {
        _tprintf(TEXT("Failed to retrieve the number of heaps with LastError %d.\n"),
                 GetLastError());
        return 1;
    }

    BytesToAllocate = NumberOfHeaps * sizeof(PHANDLE);
    aHeaps = (PHANDLE)HeapAlloc(GetProcessHeap(), 0, BytesToAllocate);
    if (aHeaps == NULL)
    {
        _tprintf(TEXT("HeapAlloc failed to allocate %d bytes.\n"),
                 BytesToAllocate);
        return 1;
    }

    HeapsLength = NumberOfHeaps;
    NumberOfHeaps = GetProcessHeaps(HeapsLength, aHeaps);
    if (NumberOfHeaps == 0)
    {
        _tprintf(TEXT("Failed to retrieve heaps with LastError %d.\n"),
                 GetLastError());
        return 1;
    }
    else if (NumberOfHeaps > HeapsLength)
    {
        _tprintf(TEXT("Another component created a heap between calls. ") \
                 TEXT("Please try again.\n"));
        return 1;
    }

    _tprintf(TEXT("Process has %d heaps.\n"), HeapsLength);
    for (HeapsIndex = 0; HeapsIndex < HeapsLength; ++HeapsIndex)
    {
        _tprintf(TEXT("Heap %d at address: %#p.\n"),
                 HeapsIndex,
                 aHeaps[HeapsIndex]);
    }

    if (HeapFree(GetProcessHeap(), 0, aHeaps) == FALSE)
    {
        _tprintf(TEXT("Failed to free allocation from default process heap.\n"));
    }

    std::cout << "Done";
    Sleep(INFINITE);
}
