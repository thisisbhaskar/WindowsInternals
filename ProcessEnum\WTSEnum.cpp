#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <wtsapi32.h>

#include <stdio.h>
#include <sstream>
#include <memory>

using namespace std;

#define MAX_USER_NAME 256
#define MAX_DOMAIN_NAME 256
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

bool GetProcessOwner(PSID p_sid, wstring & p_process_owner)
{
    auto l_continue = true;

    /* Got SID of the process owner. Get name */
    WCHAR l_user_name[MAX_USER_NAME];
    WCHAR l_domain_name[MAX_DOMAIN_NAME];
    if (l_continue)
    {
        SID_NAME_USE l_sid_type;
        DWORD l_account_size = MAX_USER_NAME;
        DWORD l_domain_size = MAX_DOMAIN_NAME;

        auto l_outcome = LookupAccountSid(nullptr, p_sid, l_user_name, &l_account_size, l_domain_name, &l_domain_size, &l_sid_type);
        if (!l_outcome)
        {
            l_continue = false;
        }
    }

    /* Form owner buffer */
    auto l_status = false;
    if (l_continue)
    {
        p_process_owner = std::wstring(l_domain_name);
        p_process_owner.append(L"\\");
        p_process_owner.append(std::wstring(l_user_name));
        l_status = true;
    }

    return l_status;
}

int main(int argc, char * argv[])
{
    WTS_PROCESS_INFO * l_info = nullptr;
    DWORD l_count = 0;

    WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE,
                          0,
                          1,
                          &l_info,
                          &l_count);


    for (DWORD l_counter = 0; l_counter < l_count; l_counter++)
    {
        PWTS_PROCESS_INFO l_entry = l_info + l_counter;
        std::cout << "PID        : " << std::to_string(l_entry->ProcessId) << endl;
        std::wcout << "Process    : " << l_entry->pProcessName << endl;
        std::cout << "Session    : " << l_entry->SessionId << endl;

        wstring p_owner;
        if (GetProcessOwner(l_entry->pUserSid, p_owner))
        {
            std::wcout << L"User       : " << p_owner << endl;
        }

        auto l_process_handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,
                                            FALSE,
                                            l_entry->ProcessId);
        if (nullptr != l_process_handle)
        {
            wstring l_process_path;
            auto l_successful = GetProcessFullPath(l_process_handle, l_process_path);
            if (l_successful)
            {
                std::wcout << L"Image Path : " << l_process_path << endl;
            }
        }
        std::cout << endl;
    }

    if (l_info)
    {
        WTSFreeMemory(l_info);
    }
}
