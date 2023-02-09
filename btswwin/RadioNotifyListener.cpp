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
	return postMessage(Message::Type::InstanceAdd, pRadioInstance, nullptr);
}

HRESULT __stdcall RadioNotifyListener::OnInstanceRemove(BSTR bstrRadioInstanceId)
{
	return postMessage(Message::Type::InstanceRemove, nullptr, bstrRadioInstanceId);
}

HRESULT __stdcall RadioNotifyListener::OnInstanceRadioChange(BSTR bstrRadioInstanceId, DEVICE_RADIO_STATE radioState)
{
	return postMessage(Message::Type::InstanceRadioChange, nullptr, bstrRadioInstanceId, radioState);
}

// Posts message to the window with Message object as lParam.
HRESULT RadioNotifyListener::postMessage(Message::Type type, IRadioInstance* radioInstance, BSTR radioInstanceId, DEVICE_RADIO_STATE radioState)
{
	auto message = new Message{type, radioInstance, radioInstanceId, radioState};
	auto hr = WIN32_EXPECT(PostMessage(m_hwnd, m_notifyMessageId, 0, (LPARAM)message));
	if(FAILED(hr)) {
		delete message;
		CString err;
		err.Format(_T(__FUNCTION__ ": PostMessage(%d) failed. Error=%d\n"), m_notifyMessageId, GetLastError());
		OutputDebugString(err.GetString());
	}
	return hr;
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
