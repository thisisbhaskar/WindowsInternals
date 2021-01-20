#include "stdafx.h"
#include <windows.h>
#include <synchapi.h>
#include <iostream>
#include <NTSecAPI.h>
#include <ntstatus.h>

using namespace std;

int main()
{
    auto l_continue = true;

    ULONG l_sessions_count = 0;
    LUID * l_sessions_ptr = nullptr;


    /* Get sessions */
    if (l_continue)
    {
        if (STATUS_SUCCESS != LsaEnumerateLogonSessions(&l_sessions_count, &l_sessions_ptr))
        {
            l_continue = false;
        }
    }

    /* Print sessions */
    if (l_continue)
    {
        for (int l_counter = 0; l_counter < l_sessions_count; l_counter++)
        {
            auto l_session_luid = l_sessions_ptr[l_counter];
            std::cout << "Session ID : " << std::hex << l_session_luid.HighPart << ":" << l_session_luid.LowPart << endl << endl;

            /* Fetch Session data */
            SECURITY_LOGON_SESSION_DATA * l_session_data;
            if (STATUS_SUCCESS == LsaGetLogonSessionData(&l_session_luid, &l_session_data))
            {
                std::cout   <<  "        LUID : " << std::hex << l_session_data->LogonId.HighPart << ":" << l_session_data->LogonId.LowPart << endl;
                std::cout   <<  "        Session : " << l_session_data->Session << endl;
                std::wcout  << L"        User : " << l_session_data->UserName.Buffer << endl;
                std::wcout  << L"        Domain : " << l_session_data->LogonDomain.Buffer << endl;
                std::wcout  << L"        LogonType : " << l_session_data->LogonType << endl;

                LsaFreeReturnBuffer((PVOID)l_session_data);
            }
            else
            {
            std::cout << "        Failed to get session data";
            }

            std::cout << endl << endl;
        }
    }

    if (l_sessions_ptr)
    {
        LsaFreeReturnBuffer((PVOID)l_sessions_ptr);
    }
}
