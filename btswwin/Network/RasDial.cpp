#include "pch.h"
#include <RasError.h>
#include "RasDial.h"
#include "ValueName.h"
#include "../Common/Assert.h"

#pragma comment(lib, "Rasapi32.lib")

static auto& logger(log4cxx::Logger::getLogger(_T("btswwin.CRasDial")));

#define RAS_ERROR(exp, error) checkRasResult(error, exp, _T(__FILE__), __LINE__)
#define RAS_ASSERT_OK(exp) do { auto _hr(RAS_EXPECT_OK(exp)); if(FAILED(_hr)) return _hr; } while(false)
#define RAS_EXPECT_OK(exp) checkRasResult(exp, _T(#exp), _T(__FILE__), __LINE__)

static CString getRasErrorString(UINT error);
static HRESULT checkRasResult(UINT error, LPCTSTR exp, LPCTSTR sourceFile, int line);

CRasDial::CRasDial()
	: m_hwnd(NULL), m_messageId(0), m_hRasconn(NULL)
{
}

HRESULT CRasDial::onNetConnected()
{
	m_connections.clear();
	m_pRasConn.reset();

	DWORD dwCb = 0;
	DWORD dwConnections = 0;
	auto error = RasEnumConnections(NULL, &dwCb, &dwConnections);

	switch(error) {
	case ERROR_BUFFER_TOO_SMALL:
		// One or more buffers are required to enumerate connections.
		// That is There's any RAS connection.
		break;
	case ERROR_SUCCESS:
		return S_OK;
	default:
		return RAS_ERROR(_T("RasEnumConnections()"), error);
	}

	auto count = (int)(dwCb / sizeof(RASCONN));
	LOG4CXX_DEBUG_FMT(logger, _T(__FUNCTION__) _T(": %d RASCONN(%d bytes)"), count, dwCb);
	HR_ASSERT(dwCb == (count * sizeof(RASCONN)), E_UNEXPECTED);

	m_pRasConn = std::make_unique<RASCONN[]>(count);
	ZeroMemory(m_pRasConn.get(), dwCb);
	m_pRasConn[0].dwSize = sizeof(RASCONN);
	if(SUCCEEDED(RAS_EXPECT_OK(
		RasEnumConnections(m_pRasConn.get(), &dwCb, &dwConnections))
	)) {
		HR_ASSERT(count == dwConnections, E_UNEXPECTED);
		for(int i = 0; i < count; i++) {
			m_connections.push_back(&m_pRasConn[i]);
		}
	}
	return S_OK;
}

HRESULT CRasDial::onNetDisconnected()
{
	m_connections.clear();
	m_pRasConn.reset();
	return S_OK;
}

bool CRasDial::isConnected(const CString& entryName) const
{
	for(auto x : m_connections) {
		if(entryName == x->szEntryName) { return true; }
	}
	return false;
}

HRESULT CRasDial::connect(HWND hwnd, UINT messageId, LPCTSTR vpnName)
{
	m_hwnd = hwnd;
	m_messageId = messageId;

	RASDIALPARAMS params = {sizeof(params)};
	_tcscpy_s(params.szEntryName, vpnName);
	BOOL gotPassword = FALSE;
	RAS_ASSERT_OK(RasGetEntryDialParams(NULL, &params, &gotPassword));

	// Value of dwCallbackId should be passed to RasDialCallback2() as 1st argument.
	params.dwCallbackId = (UINT_PTR)this;
	RAS_ASSERT_OK(RasDial(NULL, NULL, &params, 2, rasDialCallback2, &m_hRasconn));

	return S_OK;
}

DWORD CRasDial::rasDialCallback2(
	ULONG_PTR dwCallbackId, DWORD unnamedParam2, HRASCONN hrasconn, UINT uint,
	tagRASCONNSTATE state, DWORD error, DWORD exerror)
{
	static const ValueName<RASCONNSTATE> states[] = {
#define ITEM(x) { RASCS_##x, _T(#x) }
	ITEM(OpenPort),
	ITEM(PortOpened),
	ITEM(ConnectDevice),
	ITEM(DeviceConnected),
	ITEM(AllDevicesConnected),
	ITEM(Authenticate),
	ITEM(AuthNotify),
	ITEM(AuthRetry),
	ITEM(AuthCallback),
	ITEM(AuthChangePassword),
	ITEM(AuthProject),
	ITEM(AuthLinkSpeed),
	ITEM(AuthAck),
	ITEM(ReAuthenticate),
	ITEM(Authenticated),
	ITEM(PrepareForCallback),
	ITEM(WaitForModemReset),
	ITEM(WaitForCallback),
	ITEM(Projected),
#if (WINVER >= 0x400)
	ITEM(StartAuthentication),
	ITEM(CallbackComplete),
	ITEM(LogonNetwork),
#endif
	ITEM(SubEntryConnected),
	ITEM(SubEntryDisconnected),
#if (WINVER >= 0x601)
	ITEM(ApplySettings),
#endif

	ITEM(Interactive),
	ITEM(RetryAuthentication),
	ITEM(CallbackSetByCaller),
	ITEM(PasswordExpired),
#if (WINVER >= 0x500)
	ITEM(InvokeEapUI),
#endif

	ITEM(Connected),
	ITEM(Disconnected)
#undef ITEM
	};
	LOG4CXX_DEBUG_FMT(logger, _T(__FUNCTION__) _T("(%s, %d, %d)"),
		ValueToString(states, state).GetString(), error, exerror);

	((CRasDial*)dwCallbackId)->rasDialCallback(hrasconn, state, error, exerror);
	return 1;	// Further callback should be called.
}

HRESULT CRasDial::rasDialCallback(HRASCONN hrasconn, tagRASCONNSTATE state, DWORD error, DWORD exerror)
{
	if((state == RASCS_DONE) || (error != 0)) {
		if(error != 0) {
			RAS_EXPECT_OK(RasHangUp(m_hRasconn));
		}
		m_hRasconn = NULL;
		auto result = new ConnectResult(error, exerror);
		if(FAILED(WIN32_EXPECT(PostMessage(m_hwnd, m_messageId, 0, (LPARAM)result)))) {
			delete result;
		}
	}
	return S_OK;
}

CRasDial::ConnectResult::ConnectResult(DWORD error, DWORD exerror)
	: error(error), exerror(exerror)
	, errorString(error ? getRasErrorString(error).GetString() : _T(""))
{
}

/*static*/ CString getRasErrorString(UINT error)
{
	CString ret;
	TCHAR errorMsg[512];
	if(SUCCEEDED(HR_EXPECT_OK(HRESULT_FROM_WIN32(
		RasGetErrorString(error, errorMsg, ARRAYSIZE(errorMsg))
	)))) {
		ret.Format(_T("%s(%d)"), errorMsg, error);
	} else {
		ret.Format(_T("%d"), error);
	}
	return ret;
}

/*static*/ HRESULT checkRasResult(UINT error, LPCTSTR exp, LPCTSTR sourceFile, int line)
{
	auto hr = HRESULT_FROM_WIN32(error);
	if(error != ERROR_SUCCESS) {
		TCHAR msg[1000] = {0};
		_tprintf_s(msg, _T("`%s` failed. %s\n  %s:%d"),
			exp, getRasErrorString(error).GetString(), sourceFile, line
		);
		auto writer = tsm::Assert::onAssertFailedWriter
			? tsm::Assert::onAssertFailedWriter
			: tsm::Assert::defaultAssertFailedWriter;
		writer(msg);
	}
	return hr;
}
