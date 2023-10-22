#include "pch.h"
#include "Net.h"
#include "ValueName.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CNet")));

CNet::CNet()
	: m_hwnd(NULL), m_messageId(WM_USER)
	, m_cookie(0), m_cRef(0)
{
}

CNet::~CNet()
{
	stop();
}

HRESULT CNet::start(HWND hwnd, UINT messageId)
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

HRESULT CNet::stop()
{
	auto hr = S_FALSE;
	if(m_cp && m_cookie) {
		hr = HR_EXPECT_OK(m_cp->Unadvise(m_cookie));
		m_cp.Release();
	}
	return hr;
}

#pragma region Implementation of INetworkListManagerEvents

HRESULT __stdcall CNet::ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
{
	static const ValueName<NLM_CONNECTIVITY> connectivities[] = {
#define ITEM(x) { NLM_CONNECTIVITY_##x, _T(#x) }
		ITEM(DISCONNECTED),
		ITEM(IPV4_NOTRAFFIC),
		ITEM(IPV6_NOTRAFFIC),
		ITEM(IPV4_SUBNET),
		ITEM(IPV4_LOCALNETWORK),
		ITEM(IPV4_INTERNET),
		ITEM(IPV6_SUBNET),
		ITEM(IPV6_LOCALNETWORK),
		ITEM(IPV6_INTERNET)
#undef ITEM
	};

	LOG4CXX_DEBUG(logger, _T(__FUNCTION__) _T(": ") << FlagValueToString(connectivities, newConnectivity).GetString());

	return WIN32_EXPECT(PostMessage(m_hwnd, m_messageId, newConnectivity, 0L));
}

#pragma endregion

#pragma region Implementation of IUnknown

HRESULT __stdcall CNet::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
{
	static const QITAB qitab[] = {
		QITABENT(CNet, INetworkListManagerEvents),
		{0}
	};
	return QISearch(this, qitab, riid, ppvObject);
}

ULONG __stdcall CNet::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CNet::Release(void)
{
	auto cRef = InterlockedDecrement(&m_cRef);
	if(cRef == 0) {
		delete this;
	}
	return cRef;
}

#pragma endregion
