bool chkIpv4InternetConnected(void)
{
    bool retVal = false;
    HRESULT hr;

    /* Initializes the COM library for use by the calling thread */
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(SUCCEEDED(hr))
    {
        INetworkListManager* pNetListMgr;
        HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, NULL, 
                                      CLSCTX_ALL, IID_INetworkListManager, 
                                      (LPVOID*)&pNetListMgr);
        if(SUCCEEDED(hr))
        {
            NLM_CONNECTIVITY nlmCon = NLM_CONNECTIVITY_DISCONNECTED;
            hr = pNetListMgr->GetConnectivity(&nlmCon);
            if(SUCCEEDED(hr))
            {
                if (nlmCon & NLM_CONNECTIVITY_IPV4_INTERNET)
                {
                    retVal = true;
                }
            }
            pNetListMgr->Release();
        }
        CoUninitialize();
    }
    return retVal; 
}
