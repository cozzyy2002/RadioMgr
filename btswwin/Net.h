#pragma once

#include <netlistmgr.h>
#include <atlbase.h>

class CNet : INetworkListManagerEvents
{
public:
    CNet();
    ~CNet();

    HRESULT start(HWND hwnd, UINT messageId);
    HRESULT stop();

#pragma region Implementation of INetworkListManagerEvents
    virtual HRESULT STDMETHODCALLTYPE ConnectivityChanged(
        /* [in] */ NLM_CONNECTIVITY newConnectivity);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
        /* [in] */ REFIID riid,
        /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);
#pragma endregion

protected:
    NLM_CONNECTIVITY m_connectifity;
    CComPtr<INetworkListManager> m_mgr;
    HWND m_hwnd;
    UINT m_messageId;
    CComPtr<IConnectionPoint> m_cp;
    DWORD m_cookie;
    ULONG m_cRef;
};
