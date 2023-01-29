#include "pch.h"
#include "framework.h"

#include "RadioNotifyListener.h"
#include "../Common/Assert.h"

HRESULT RadioNotifyListener::advise(IConnectionPoint* cp)
{
	m_cp = cp;
	return cp ?
		cp->Advise(this, &m_cookie) :
		E_POINTER;
}

HRESULT RadioNotifyListener::unadvise()
{
	return (m_cp && m_cookie)
		? m_cp->Unadvise(m_cookie)
		: S_FALSE;
}

#pragma region Implementation of IMediaRadioManagerNotifySink
HRESULT __stdcall RadioNotifyListener::OnInstanceAdd(IRadioInstance* pRadioInstance)
{
	return E_NOTIMPL;
}

HRESULT __stdcall RadioNotifyListener::OnInstanceRemove(BSTR bstrRadioInstanceId)
{
	return E_NOTIMPL;
}

HRESULT __stdcall RadioNotifyListener::OnInstanceRadioChange(BSTR bstrRadioInstanceId, DEVICE_RADIO_STATE radioState)
{
	return WIN32_EXPECT(PostMessage(m_hwnd, m_notifyMessageId, radioState, (LPARAM)bstrRadioInstanceId));
}
#pragma endregion


#pragma region Implementation of IUnknown
HRESULT __stdcall RadioNotifyListener::QueryInterface(REFIID riid, void** ppvObject)
{
	static const QITAB qitab[] = {
		QITABENT(RadioNotifyListener, IMediaRadioManagerNotifySink),
		{0}
	};
	return QISearch(this, qitab, riid, ppvObject);
}

ULONG __stdcall RadioNotifyListener::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall RadioNotifyListener::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	if(cRef == 0) {
		delete this;
	}
	return cRef;
}
#pragma endregion
