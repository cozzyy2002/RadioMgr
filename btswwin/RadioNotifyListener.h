#pragma once

#include <RadioMgr.h>

#include <atlbase.h>


class RadioNotifyListener : public IMediaRadioManagerNotifySink
{
public:
	RadioNotifyListener(HWND hwnd, UINT notifyMessageId)
		: m_cookie(0)
		, m_hwnd(hwnd), m_notifyMessageId(notifyMessageId)
		, m_cRef(0) {}
	~RadioNotifyListener() {}

	HRESULT advise(IConnectionPoint*);
	HRESULT unadvise();

protected:
	CComPtr<IConnectionPoint> m_cp;
	DWORD m_cookie;

#pragma region Implementation of IMediaRadioManagerNotifySink
public:
	virtual HRESULT STDMETHODCALLTYPE OnInstanceAdd(
		/* [in] */ IRadioInstance* pRadioInstance);
	virtual HRESULT STDMETHODCALLTYPE OnInstanceRemove(
		/* [string][in] */ BSTR bstrRadioInstanceId);
	virtual HRESULT STDMETHODCALLTYPE OnInstanceRadioChange(
		/* [string][in] */ BSTR bstrRadioInstanceId,
		/* [in] */ DEVICE_RADIO_STATE radioState);
protected:
	HWND m_hwnd;
	UINT m_notifyMessageId;
#pragma endregion

#pragma region Implementation of IUnknown
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);
protected:
	volatile ULONG m_cRef;
#pragma endregion
};
