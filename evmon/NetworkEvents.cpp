#include "NetworkEvents.h"
#include "../Common/Assert.h"

NetworkListManagerEvents::NetworkListManagerEvents()
	: m_hwnd(NULL), m_messageId(WM_USER)
	, m_cRef(0)
{
}

HRESULT NetworkListManagerEvents::start(HWND hwnd, UINT messageId)
{
	m_hwnd = hwnd;
	m_messageId = messageId;

	return E_NOTIMPL;
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
	static QITAB qitab[] = {
		QITABENT(NetworkListManagerEvents, INetworkListManagerEvents),
		{0}
	};
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
