#pragma once

#define EXPORTING_DLL

extern "C"
{
#ifdef EXPORTING_DLL
    extern __declspec(dllexport) void HelloWorld();
#else
    extern __declspec(dllimport) void HelloWorld();
#endif
}