#pragma once

#include <Ras.h>

#pragma comment(lib, "Rasapi32.lib")

class CRasDial
{
public:
	CRasDial();

	// Returns RAS connection(s) if any.
	int getConnection(std::unique_ptr<RASCONN[]>* pRasConn = nullptr) const;

	// Returns true if any VPN is connected.
	bool isConnected() const { return (0 < getConnection()); }
	bool isConnecting() const { return m_hRasconn ? true : false; }

	// Returns whether connect() can be called or not.
	// This method might return true even if the network is not available.
	bool canConnect() const { return !isConnected() && !isConnecting(); }

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

	static DWORD rasDialCallback2(
		ULONG_PTR dwCallbackId, DWORD unnamedParam2, HRASCONN hrasconn, UINT uint,
		tagRASCONNSTATE state, DWORD error, DWORD exerror);
	HRESULT rasDialCallback(HRASCONN hrasconn, tagRASCONNSTATE state, DWORD error, DWORD exerror);
};
