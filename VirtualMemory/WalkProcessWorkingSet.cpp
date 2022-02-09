QueryWorkingSet


DWORD CalculateWSPrivate(DWORD processID)
{
    DWORD dSharedPages = 0;
    DWORD dPrivatePages = 0; 
    DWORD dPageTablePages = 0;
  
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );

    if ( !hProcess ) return 0;

    if ( !QueryWorkingSet(hProcess, dWorkingSetPages, sizeof(dWorkingSetPages)) ) 
    {
        CloseHandle( hProcess );
        return 0;
    }
  
    DWORD dPages = dWorkingSetPages[0];
    for ( DWORD i = 1; i <= dPages; i++ )
    {
      DWORD dCurrentPageStatus = 0; 
      DWORD dCurrentPageAddress;
      DWORD dNextPageAddress;
      DWORD dNextPageFlags;
      DWORD dPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
      DWORD dPageFlags = dWorkingSetPages[i] & 0x00000FFF;   
    }

    CloseHandle( hProcess );
}
