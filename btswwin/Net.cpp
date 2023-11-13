#include "pch.h"
#include "Net.h"
#include "ValueName.h"
#include "../Common/Assert.h"

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CNet")));

// Class to implement INetworkListManagerEvents interface.
class NetworkListManagerEvents : public INetworkListManagerEvents
{
public:
	NetworkListManagerEvents(HWND hwnd, UINT messageId)
		: m_connectifity(NLM_CONNECTIVITY_DISCONNECTED)
		, m_hwnd(hwnd), m_messageId(messageId), m_cRef(0) {}

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
	HWND m_hwnd;
	UINT m_messageId;
	ULONG m_cRef;
};

CNet::CNet()
	: m_cookie(0)
{
}

CNet::~CNet()
{
	stop();
}

HRESULT CNet::start(HWND hwnd, UINT messageId)
{
	HR_ASSERT_OK(m_mgr.CoCreateInstance(CLSID_NetworkListManager));
	CComPtr<IConnectionPointContainer> cpc;
	HR_ASSERT_OK(m_mgr.QueryInterface(&cpc));
	HR_ASSERT_OK(cpc->FindConnectionPoint(IID_INetworkListManagerEvents, &m_cp));

	m_events = new NetworkListManagerEvents(hwnd, messageId);
	HR_ASSERT_OK(m_cp->Advise(m_events, &m_cookie));
	return S_OK;
}

HRESULT CNet::stop()
{
	auto hr = S_FALSE;
	if(m_cp && m_cookie) {
		hr = HR_EXPECT_OK(m_cp->Unadvise(m_cookie));
		m_cp.Release();
	}
	m_events.Release();
	return hr;
}

#pragma region Implementation of INetworkListManagerEvents

HRESULT __stdcall NetworkListManagerEvents::ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
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

	LOG4CXX_DEBUG_FMT(logger, _T(__FUNCTION__) _T(": %s"), FlagValueToString(connectivities, newConnectivity).GetString());

	static const auto mask = NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET;
	auto isCurrentConnected = ((m_connectifity & mask) != NLM_CONNECTIVITY_DISCONNECTED);
	auto isNewConnected = ((newConnectivity & mask) != NLM_CONNECTIVITY_DISCONNECTED);
	if(isCurrentConnected ^ isNewConnected) {
		WIN32_ASSERT(PostMessage(m_hwnd, m_messageId, (WPARAM)(isNewConnected ? 1 : 0), 0L));
	}

	m_connectifity = newConnectivity;
	return S_OK;
}

#pragma endregion

#pragma region Implementation of IUnknown

HRESULT __stdcall NetworkListManagerEvents::QueryInterface(REFIID riid, _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject)
{
	static const QITAB qitab[] = {
		QITABENT(CNet, INetworkListManagerEvents),
		{0}
	};
	return QISearch(this, qitab, riid, ppvObject);
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
