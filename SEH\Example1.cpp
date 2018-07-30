#include "stdafx.h"
#include <afxwin.h>
#include <Windows.h>
#include <string>
#include <memory>
#include <iostream>
#include <ntstatus.h>
#include <synchapi.h>
#include <strsafe.h>
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <stdio.h>
#include <tchar.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <Softpub.h>
#include <string>

int func(unsigned long p_code, struct _EXCEPTION_POINTERS* p_info)
{
    std::cout << p_code;
    return EXCEPTION_EXECUTE_HANDLER;
}

int _tmain(int argc, TCHAR *argv[])
{

    __try
    {
        int a = 10;
        int b = 0;
        int x = a/b;

    } __except(func(GetExceptionCode(), GetExceptionInformation()))
    {
        std::cout << "handler";
    }
}

