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

bool EnablePrivilege(wstring const & p_privilege, bool l_enable_privilege /*= TRUE*/)
{
    auto l_enabled = false;

    HANDLE l_token_handle;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &l_token_handle))
    {
        /* Privilege */
        TOKEN_PRIVILEGES l_token_privileges = { 1 };

        if (LookupPrivilegeValue(nullptr, p_privilege.c_str(), &l_token_privileges.Privileges[0].Luid))
        {
            l_token_privileges.Privileges[0].Attributes = l_enable_privilege ? SE_PRIVILEGE_ENABLED : 0;
            AdjustTokenPrivileges(l_token_handle, FALSE, &l_token_privileges, sizeof(l_token_privileges), nullptr, nullptr);
            l_enabled = (ERROR_SUCCESS == GetLastError());
        }

        CloseHandle(l_token_handle);
    }

    return l_enabled;
}

std::string getSessionStateString(DWORD p_session_id)
{
    switch (p_session_id)
    {
    case WTSActive: return "WTSActive";
    case WTSConnected: return "WTSConnected";
    case WTSConnectQuery: return "WTSConnectQuery";
    case WTSShadow: return "WTSShadow";
    case WTSDisconnected: return "WTSDisconnected";
    case WTSIdle: return "WTSIdle";
    case WTSListen: return "WTSListen";
    case WTSReset: return "WTSReset";
    case WTSDown: return "WTSDown";
    case WTSInit: return "WTSInit";
    }
    return "Unknown";
}

int main(int argc, char * argv[])
{

    WTS_SESSION_INFO * l_sessions;
    DWORD l_count;
    if (!WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &l_sessions, &l_count))
    {
        return -1;
    }

    for (DWORD i = 0; i < l_count; i++)
    {
        wprintf(L"  *   SessionID: %d \n", l_sessions[i].SessionId);
        wprintf(L"      WinStation: %s\n", l_sessions[i].pWinStationName);
        printf("      State: %s\n", getSessionStateString(l_sessions[i].State).c_str());

        WCHAR *l_session_user;
        DWORD l_bytes = 0;
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, l_sessions[i].SessionId, WTSUserName, &l_session_user, &l_bytes))
        {
            wprintf(L"      User: %s\n", l_session_user);
            WTSFreeMemory(l_session_user);
        }

        WCHAR *l_user_domain;
        l_bytes = 0;
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, l_sessions[i].SessionId, WTSDomainName, &l_user_domain, &l_bytes))
        {
            wprintf(L"      Domain: %s\n", l_user_domain);
            WTSFreeMemory(l_user_domain);
        }

        WCHAR *l_client_name;
        l_bytes = 0;
        if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, l_sessions[i].SessionId, WTSClientName, &l_client_name, &l_bytes))
        {
            wprintf(L"      ClientName: %s\n", l_client_name);
            WTSFreeMemory(l_client_name);
        }

        std::cout << endl;

    }

    WTSFreeMemory(l_sessions);
}


/*
typedef enum _WTS_CONNECTSTATE_CLASS {
[0] WTSActive,
[1] WTSConnected,
[2] WTSConnectQuery,
[3] WTSShadow,
[4] WTSDisconnected,
[5] WTSIdle,
[6] WTSListen,
[7] WTSReset,
[8] WTSDown,
[9] WTSInit
} WTS_CONNECTSTATE_CLASS;
*/
