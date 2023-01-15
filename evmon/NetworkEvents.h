#pragma once

#include <netlistmgr.h>
#include <atlbase.h>

class NetworkListManagerEvents : INetworkListManagerEvents
{
public:
    NetworkListManagerEvents();
    HRESULT start(HWND hwnd, UINT messageId);

    virtual HRESULT STDMETHODCALLTYPE ConnectivityChanged(
        /* [in] */ NLM_CONNECTIVITY newConnectivity);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

protected:
    CComPtr<INetworkListManager> m_mgr;
    HWND m_hwnd;
    UINT m_messageId;
    ULONG m_cRef;
};

#define HANDLE_WM_NETWORK_CONNECTIVITYCHANGED(hwnd, wParam, lParam, fn) (void)(fn)((hwnd), (NLM_CONNECTIVITY)(wParam)), 0L
