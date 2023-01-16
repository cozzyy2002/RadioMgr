#include "NetworkEvents.h"
#include "../Common/Assert.h"

NetworkListManagerEvents::NetworkListManagerEvents()
	: m_hwnd(NULL), m_messageId(WM_USER)
	, m_cookie(0), m_cRef(0)
{
}

HRESULT NetworkListManagerEvents::start(HWND hwnd, UINT messageId)
{
	m_hwnd = hwnd;
	m_messageId = messageId;

	HR_ASSERT_OK(m_mgr.CoCreateInstance(CLSID_NetworkListManager));
	CComPtr<IConnectionPointContainer> cpc;
	HR_ASSERT_OK(m_mgr.QueryInterface(&cpc));
	HR_ASSERT_OK(cpc->FindConnectionPoint(IID_INetworkListManagerEvents, &m_cp));
	HR_ASSERT_OK(m_cp->Advise(this, &m_cookie));
	return S_OK;
}

HRESULT NetworkListManagerEvents::stop()
{
	auto hr = S_FALSE;
	if(m_cp && m_cookie) {
		hr = HR_EXPECT_OK(m_cp->Unadvise(m_cookie));
	}
	return hr;
}

#pragma region Implementation of INetworkListManagerEvents

HRESULT __stdcall NetworkListManagerEvents::ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
{
	return WIN32_EXPECT(PostMessage(m_hwnd, m_messageId, newConnectivity, 0L));
}

#pragma endregion

#pragma region Implementation of IUnknown

HRESULT __stdcall NetworkListManagerEvents::QueryInterface(REFIID riid, void** ppvObject)
{
	static const QITAB qitab[] = {
		QITABENT(NetworkListManagerEvents, INetworkListManagerEvents),
		{0}
	};

	TCHAR strGuid[50];
	StringFromGUID2(riid, strGuid, ARRAYSIZE(strGuid));
	_tprintf_s(_T(__FUNCTION__ ": %s\n"), strGuid);
	return HR_EXPECT_OK(QISearch(this, qitab, riid, ppvObject));
}

ULONG __stdcall NetworkListManagerEvents::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall NetworkListManagerEvents::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	if(cRef == 0) {
		delete this;
	}
	return cRef;
}

#pragma endregion
