#pragma once

#include <Ras.h>

#pragma comment(lib, "Rasapi32.lib")

class CRasDial
{
public:
	CRasDial();

	bool canConnect() const;
	HRESULT connect(HWND hwnd, UINT messageId, LPCTSTR vpnName);
	HRESULT cancelConnect();

protected:
	HWND m_hwnd;
	UINT m_messageId;

	static DWORD rasDialCallback2(ULONG_PTR dwCallbackId, DWORD unnamedParam2,
									HRASCONN hrasconn, UINT uint, tagRASCONNSTATE state, DWORD error, DWORD exerror);
	HRESULT rasDialCallback(HRASCONN hrasconn, tagRASCONNSTATE state, DWORD error, DWORD exerror);
};
