    BOOL LaunchProcessAsUser(const char* pcszProcessPath,
                             char* pszProcessArg,
                             BOOLEAN bWaitToFinish,
                             DWORD* pdwOutVal,
                             DWORD* pdwError)
    {
        BOOL boRet;
        HRESULT hr;
        DWORD dwRet;
        int conSessId = 0;
        DWORD dwCount = 0;
        HANDLE userToken = NULL;
        LPVOID pEnvironment = NULL;
        HANDLE processToken = NULL;
        HANDLE impersonationToken = NULL;
        PWTS_SESSION_INFO pSessionInfo = 0;
        TOKEN_PRIVILEGES oldTokenPrivileges = { 0 };
        PROCESS_INFORMATION processInformation = { 0 };

        // Validate parameters.
        if ((NULL == pcszProcessPath && NULL == pszProcessArg) || NULL == pdwOutVal || NULL == pdwError)
        {
            return FALSE;
        }

        *pdwOutVal = 0;
        *pdwError = 0;

        __try
        {

          boRet = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &processToken);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            LUID luid;
            boRet = LookupPrivilegeValue(NULL, _T("SeTcbPrivilege"), &luid);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            TOKEN_PRIVILEGES adjTokenPrivileges = { 0 };
            adjTokenPrivileges.PrivilegeCount = 1;
            adjTokenPrivileges.Privileges[0].Luid = luid;
            adjTokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            DWORD dwOldTPLen;
            boRet = AdjustTokenPrivileges(processToken, FALSE, &adjTokenPrivileges, sizeof(TOKEN_PRIVILEGES), &oldTokenPrivileges, &dwOldTPLen);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            // Get the list of all terminal sessions 
            boRet = WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            // look over obtained list in search of the active session.
            for (DWORD dwLoop = 0; dwLoop < dwCount; ++dwLoop)
            {
                WTS_SESSION_INFO si = pSessionInfo[dwLoop];
                if (WTSActive == si.State)
                {
                    // If the current session is active SPA store its ID.
                    conSessId = si.SessionId;
                    break;
                }
            }

            if (pSessionInfo)
            {
                WTSFreeMemory(pSessionInfo);
            }

            boRet = WTSQueryUserToken(conSessId, &impersonationToken);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            boRet = DuplicateTokenEx(impersonationToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &userToken);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            STARTUPINFO si = { 0 };
            si.cb = sizeof(STARTUPINFO);
            si.lpDesktop = _T("winsta0\\default");

            boRet = CreateEnvironmentBlock(&pEnvironment, userToken, TRUE);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            boRet = CreateProcessAsUser(userToken,
                                        _T(pcszProcessPath),
                                        _T(pszProcessArg),
                                        NULL,
                                        NULL,
                                        FALSE,
                                        CREATE_UNICODE_ENVIRONMENT,
                                        pEnvironment,
                                        NULL,
                                        &si,
                                        &processInformation);
            if (!boRet)
            {
                *pdwError = GetLastError();
                return FALSE;
            }

            if (bWaitToFinish)
            {
                // Successfully created the process. Wait for it to finish.
                dwRet = WaitForSingleObject(processInformation.hProcess, INFINITE);
                switch (dwRet)
                {
                case WAIT_OBJECT_0:
                {
                    if (!GetExitCodeProcess(processInformation.hProcess, pdwOutVal))
                    {
                        *pdwError = GetLastError();
                        return FALSE;
                    }
                }
                break;

                case WAIT_FAILED:
                    *pdwError = GetLastError();
                    LGMLOG(ERROR, "Wait failed. Error code (%ld).", *pdwError);
                    return FALSE;

                default:
                    *pdwError = 0;
                    break;
                }
            }
            else
            {
                // Store PID of new process.
                *pdwOutVal = processInformation.dwProcessId;
            }
        }
        __finally
        {
            if (processInformation.hThread)
            {
                CT_CloseHandle(processInformation.hThread);
            }
          
            if (processInformation.hProcess)
            {
                CT_CloseHandle(processInformation.hProcess);
            }
          
            if (pEnvironment)
            {
                DestroyEnvironmentBlock(pEnvironment);
            }
          
            if (userToken)
            {
                CT_CloseHandle(userToken);
            }
          
            if (impersonationToken)
            {
                CT_CloseHandle(impersonationToken);
            }
          
            if (processToken)
            {
                AdjustTokenPrivileges(processToken, FALSE, &oldTokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
                CT_CloseHandle(processToken);
            }
        }

        return TRUE;
    }
}
