#pragma once

#include <netlistmgr.h>
#include <atlbase.h>

class CNet
{
public:
    CNet();
    ~CNet();

    HRESULT start(HWND hwnd, UINT messageId);
    HRESULT stop();

protected:
    CComPtr<INetworkListManager> m_mgr;
    CComPtr<IConnectionPoint> m_cp;
    CComPtr<INetworkListManagerEvents> m_events;
    DWORD m_cookie;
};
