#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <processthreadsapi.h>
#include <processthreadsapi.h>

#include <stdio.h>
#include <sstream>
#include <memory>

using namespace std;

#define MAX_PATH_LEN 32767

/**
* @brief Get full-path for the given possible short-path
*/
bool GetLongPathFromPossibleShortPath(wstring const & p_possible_short_path, wstring & p_fullpath)
{
    auto l_status = false;
    auto l_continue = true;

    if (p_possible_short_path.empty())
    {
        l_status = false;
        l_continue = false;
    }

    if (l_continue)
    {
        TCHAR l_buffer[MAX_PATH_LEN];
        auto l_ret = GetLongPathName(p_possible_short_path.c_str(), l_buffer, MAX_PATH_LEN);
        if (l_ret == 0 || l_ret >= MAX_PATH_LEN)
        {
            l_status = false;
        }
        else
        {
            l_status = true;
            p_fullpath = std::wstring(l_buffer, l_ret);
        }
    }

    return l_status;
}

bool GetProcessFullPath(HANDLE const & p_process_handle, wstring & p_fullpath)
{
    auto l_successful = false;

#if defined(_WINDOWS_XP_COMPATIBLE_)
    l_successful = GetModuleFullPath(p_process_handle, nullptr, p_fullpath);
#else

    DWORD  l_buffer_size_in_chars = MAX_PATH;
    LPTSTR l_buffer = nullptr;

    while (true)
    {
        /* Allocate memory for buffer */
        l_buffer = reinterpret_cast<LPTSTR>(::malloc(sizeof(TCHAR) * l_buffer_size_in_chars));

        auto l_outcome = ::QueryFullProcessImageName(p_process_handle, 0, l_buffer, &l_buffer_size_in_chars);
        if (l_outcome)
        {
            p_fullpath = std::wstring(l_buffer);

            /* Try to get long path */
            wstring l_long_path;
            if (GetLongPathFromPossibleShortPath(p_fullpath, l_long_path))
            {
                if (p_fullpath != l_long_path)
                {
                    p_fullpath = l_long_path;
                }
            }

            /* Break as we are done */
            l_successful = true;
            break;
        }
        else
        {
            auto l_error = GetLastError();
            if (ERROR_INSUFFICIENT_BUFFER == l_error)
            {
                /* Deallocate current buffer and increase the size */
                if (nullptr != l_buffer)
                {
                    free(l_buffer);
                    l_buffer = nullptr;
                }

                /* Increase the buffer size */
                l_buffer_size_in_chars += MAX_PATH;
            }
            else
            {
                /* Failed with unknown error */
                break;
            }
        }
    }

    if (nullptr != l_buffer)
    {
        free(l_buffer);
        l_buffer = nullptr;
    }

#endif
    return l_successful;
}
int main(int argc, char * argv[])
{

    auto l_continue = true;
   
    SC_HANDLE l_scm_handle = nullptr;
    if (l_continue)
    {
        l_scm_handle = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
        if (nullptr == l_scm_handle)
        {
            l_continue = false;
        }
    }

    char l_buffer[2 * 1024];
    DWORD l_bytes_needed = 0;
    DWORD l_services_count = 0;
    DWORD l_resume_handle = 0;
    DWORD l_global_count = 0;
    while (true)
    {
        auto l_break = false;
        if (l_continue)
        {
            auto l_status = EnumServicesStatusEx(l_scm_handle,
                                                 SC_ENUM_PROCESS_INFO,
                                                 SERVICE_WIN32,
                                                 SERVICE_STATE_ALL,
                                                 (LPBYTE)(&l_buffer),
                                                 sizeof(l_buffer),
                                                 &l_bytes_needed,
                                                 &l_services_count,
                                                 &l_resume_handle,
                                                 nullptr);
            if (!l_status)
            {
                if (ERROR_MORE_DATA != GetLastError())
                {
                    l_continue = false;
                }
            }
            else
            {
                l_break = true;
            }
        }

        if (l_continue)
        {
            for (int l_count = 0; l_count < l_services_count; l_count++)
            {
                LPENUM_SERVICE_STATUS_PROCESS l_ptr = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESS>(l_buffer + sizeof(ENUM_SERVICE_STATUS_PROCESS) * l_count);
                std::cout << "Service " << ++l_global_count << endl;
                std::wcout << L"\rDisplay Name   : " << l_ptr->lpDisplayName << endl;
                std::wcout << L"\rService Name   : " << l_ptr->lpServiceName << endl;
                std::wcout << L"\rPID            : " << l_ptr->ServiceStatusProcess.dwProcessId << endl;


                if (l_ptr->ServiceStatusProcess.dwProcessId > 0)
                {
                    auto l_process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                                        FALSE,
                                                        l_ptr->ServiceStatusProcess.dwProcessId);
                    if (nullptr != l_process_handle)
                    {
                        wstring l_process_path;
                        auto l_successful = GetProcessFullPath(l_process_handle, l_process_path);
                        if (l_successful)
                        {
                            std::wcout << L"\rImage Path     : " << l_process_path << endl;
                        }
                    }
                }

                auto l_service_handle = OpenService(l_scm_handle,
                                                    l_ptr->lpServiceName,
                                                    SC_MANAGER_ALL_ACCESS);
                if (nullptr != l_service_handle)
                {
                    char l_service_info[8 * 1024];
                    DWORD l_bytes_needed = 0;
                    LPQUERY_SERVICE_CONFIG l_ptr = reinterpret_cast<LPQUERY_SERVICE_CONFIG>(&l_service_info);
                    if (QueryServiceConfig(l_service_handle,
                                           l_ptr,
                                           8 * 1024,
                                           &l_bytes_needed))
                    {
                        std::wcout << L"\rBinary Path    : " << l_ptr->lpBinaryPathName << endl;
                    }
                }

                std::cout << endl << endl;
            }
        }

        if (l_break)
        {
            break;
        }
    }

    if (nullptr != l_scm_handle)
    {
        CloseServiceHandle(l_scm_handle);
    }

}
