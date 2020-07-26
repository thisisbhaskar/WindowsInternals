#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <wtsapi32.h>
#include <ntsecapi.h>
#include <processthreadsapi.h>

#include <stdio.h>
#include <sstream>
#include <memory>

using namespace std;

#define MAX_USER_NAME 256
#define MAX_DOMAIN_NAME 256

int main(int argc, char * argv[])
{
    BOOL l_status;
    TOKEN_TYPE l_token_type = TokenPrimary;
    SECURITY_IMPERSONATION_LEVEL l_imper_level = SecurityAnonymous;
    DWORD l_error;
    DWORD l_return_length = 0;

    HANDLE l_process_token = nullptr;
    SetLastError(0);
    l_status = OpenProcessToken(GetCurrentProcess(),
                                TOKEN_QUERY | TOKEN_DUPLICATE,
                                &l_process_token);
    l_error = GetLastError();

    HANDLE l_duplicate_token = nullptr;
    if (!l_status)
    {
        return -1;
    }

    l_return_length = 0;
    SetLastError(0);
    GetTokenInformation(l_process_token,
                        TokenType,
                        &l_token_type,
                        sizeof(l_token_type),
                        &l_return_length);
    l_error = GetLastError();

    l_return_length = 0;
    SetLastError(0);
    GetTokenInformation(l_process_token,
                        TokenImpersonationLevel,
                        &l_imper_level,
                        sizeof(l_imper_level),
                        &l_return_length);
    l_error = GetLastError();

    l_return_length = 0;
    SetLastError(0);
    l_status = DuplicateToken(l_process_token,
                              SecurityAnonymous,
                              &l_duplicate_token);
    l_error = GetLastError();

    l_return_length = 0;
    SetLastError(0);
    GetTokenInformation(l_duplicate_token,
                        TokenType,
                        &l_token_type,
                        sizeof(l_token_type),
                        &l_return_length);
    l_error = GetLastError();

    l_return_length = 0;
    SetLastError(0);
    GetTokenInformation(l_duplicate_token,
                        TokenImpersonationLevel,
                        &l_imper_level,
                        sizeof(l_imper_level),
                        &l_return_length);
    l_error = GetLastError();

    SetLastError(0);
    ImpersonateSelf(SecurityAnonymous);
    l_error = GetLastError();

    HANDLE l_thread_token = nullptr;
    SetLastError(0);
    l_status = OpenThreadToken(GetCurrentThread(),
                               TOKEN_QUERY,
                               TRUE,
                               &l_thread_token);
    l_error = GetLastError();

    if (l_status)
    {
        std::cout << "We have tokens.";

        l_return_length = 0;
        SetLastError(0);
        GetTokenInformation(l_thread_token,
                            TokenType,
                            &l_token_type,
                            sizeof(l_token_type),
                            &l_return_length);
        l_error = GetLastError();

        l_return_length = 0;
        SetLastError(0);
        GetTokenInformation(l_thread_token,
                            TokenImpersonationLevel,
                            &l_imper_level,
                            sizeof(l_imper_level),
                            &l_return_length);
        l_error = GetLastError();

        SetLastError(0);
        DuplicateToken(l_thread_token,
                       SecurityIdentification,
                       &l_duplicate_token);
        l_error = GetLastError();

        l_return_length = 0;
        SetLastError(0);
        GetTokenInformation(l_duplicate_token,
                            TokenType,
                            &l_token_type,
                            sizeof(l_token_type),
                            &l_return_length);
        l_error = GetLastError();

        l_return_length = 0;
        SetLastError(0);
        GetTokenInformation(l_duplicate_token,
                            TokenImpersonationLevel,
                            &l_imper_level,
                            sizeof(l_imper_level),
                            &l_return_length);
        l_error = GetLastError();
    }
    else
    {
        std::cout << "OpenThreadToken failed. Reason : " << std::to_string(GetLastError());
    }
}
