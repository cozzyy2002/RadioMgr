#pragma once

#include <Ras.h>

#pragma comment(lib, "Rasapi32.lib")

class CRasDial
{
public:
	CRasDial();

	// Called when Network is connected.
	HRESULT onNetConnected();
	// Called when Network is disconnected.
	HRESULT onNetDisconnected();

	// Returns RAS connection(s).
	const std::vector<const RASCONN*>& getConnections() const { return m_connections; }

	// Returns true if specified connection is available.
	bool isConnected(const CString& entryName) const;
	bool isConnecting() const { return m_hRasconn ? true : false; }

	struct ConnectResult {
		explicit ConnectResult(DWORD error, DWORD exerror);

		bool success() const { return error == 0; }
		const DWORD error;
		const DWORD exerror;
		const CString errorString;
	};

	HRESULT connect(HWND hwnd, UINT messageId, LPCTSTR vpnName);

protected:
	HWND m_hwnd;
	UINT m_messageId;
	HRASCONN m_hRasconn;
	std::unique_ptr<RASCONN[]> m_pRasConn;
	std::vector<const RASCONN*> m_connections;

	static DWORD rasDialCallback2(
		ULONG_PTR dwCallbackId, DWORD unnamedParam2, HRASCONN hrasconn, UINT uint,
		tagRASCONNSTATE state, DWORD error, DWORD exerror);
	HRESULT rasDialCallback(HRASCONN hrasconn, tagRASCONNSTATE state, DWORD error, DWORD exerror);
};
